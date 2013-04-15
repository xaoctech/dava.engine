#include "Scene/System/ModifSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/SelectionSystem.h"

#include "Scene/EntityGroup.h"
#include "Scene/SceneEditorProxy.h"

EntityModificationSystem::EntityModificationSystem(DAVA::Scene * scene, SceneCollisionSystem *colSys, SceneCameraSystem *camSys, HoodSystem *hoodSys)
	: DAVA::SceneSystem(scene)
	, collisionSystem(colSys)
	, cameraSystem(camSys)
	, hoodSystem(hoodSys)
	, inModifState(false)
	, modified(false)
	, curMode(EM_MODE_MOVE)
	, curAxis(EM_AXIS_Z)
{ }

EntityModificationSystem::~EntityModificationSystem()
{ }

void EntityModificationSystem::SetModifAxis(int axis)
{
	if(axis != EM_AXIS_NONE)
	{
		curAxis = axis;
		hoodSystem->SetSelectedAxis(axis);
	}
}

int EntityModificationSystem::GetModifAxis() const
{
	return curAxis;
}

void EntityModificationSystem::Update(DAVA::float32 timeElapsed)
{ }

void EntityModificationSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	if(NULL != collisionSystem)
	{
		// current selected entities
		SceneSelectionSystem *selectionSystem = ((SceneEditorProxy *) GetScene())->selectionSystem;
		const EntityGroup *selectedEntities = selectionSystem->GetSelection();

		// remember current cursor point, when looking from current camera
		DAVA::Vector3 camPosition = cameraSystem->GetCameraPosition();
		DAVA::Vector3 camToPointDirection = cameraSystem->GetPointDirection(event->point);

		// if we are not in modification state, try to find some selected item
		// that have mouse cursor at the top of it
		if(!inModifState)
		{
			bool modifCanStart = false;

			// we can start modification only if mouse is over hood
			// on mouse is over one of currently selected items
			int mouseOverAxis = hoodSystem->GetPassingAxis();
			if(mouseOverAxis != EM_AXIS_NONE)
			{
				// allow starting modification
				modifCanStart = true;
			}
			else
			{
				// send this ray to collision system and get collision objects
				const EntityGroup *collisionEntities = collisionSystem->RayTestFromCamera();

				// check if one of got collision objects is intersected with selected items
				// if so - we can start modification
				if(NULL != GetFirstIntersectedEntity(selectedEntities, collisionEntities))
				{
					modifCanStart = true;
				}
			}

			// can we start modification???
			if(modifCanStart)
			{
				// TODO:
				// emit signal about mouse is over selected object
				// to set specific cursor icon
				// ...
				// 

				if(DAVA::UIEvent::PHASE_BEGAN == event->phase)
				{
					if(event->tid == DAVA::UIEvent::BUTTON_1)
					{
						// go to modification state
						inModifState = true;

						// select current hood axis as active
						SetModifAxis(mouseOverAxis);

						// set entities to be modified
						BeginModification(selectedEntities);

						// init some values, needed for modifications
						modifStartPos3d = CamCursorPosToModifPos(camPosition, camToPointDirection, modifEntitiesCenter);
						modifStartPos2d = event->point;
					}
				}
			}
		}
		// or we are already in modification state
		else
		{
			// phase still continue
			if(event->phase == DAVA::UIEvent::PHASE_DRAG)
			{
				switch (curMode)
				{
				case EM_MODE_MOVE:
					{
						Move(CamCursorPosToModifPos(camPosition, camToPointDirection, modifEntitiesCenter));
						modified = true;
					}
					break;
				case EM_MODE_ROTATE:
					{
						Rotate(event->point);
						modified = true;
					}
					break;
				case EM_MODE_SCALE:
					{
						Scale(event->point);
						modified = true;
					}
					break;
				default:
					break;
				}

				if(modified)
				{
					// say to selection system, that selected items were modified
					selectionSystem->SelectedItemsWereModified();

					// lock hood, so it wont process ui events, wont calc. scale depending on it current position
					hoodSystem->Lock();
				}
			}
			// phase ended
			else if(event->phase == DAVA::UIEvent::PHASE_ENDED)
			{
				if(event->tid == DAVA::UIEvent::BUTTON_1)
				{
					// TODO:
					// apply modification commands
					// ...
					// 

					EndModification();
					inModifState = false;
					modified = false;

					hoodSystem->Unlock();
					selectionSystem->UpdateHoodPos();
				}
			}
		}
	}
}

void EntityModificationSystem::Draw()
{ }

void EntityModificationSystem::BeginModification(const EntityGroup *entities)
{
	// clear any priv. selection
	EndModification();

	if(NULL != entities)
	{
		for(size_t i = 0; i < entities->Size(); ++i)
		{
			DAVA::Entity *en = entities->Get(i);
			if(NULL != en)
			{
				EntityToModify etm;
				etm.entity = en;
				etm.originalCenter = en->GetWorldTransform().GetTranslationVector();
				etm.originalTransform = en->GetLocalTransform();
				etm.moveToZeroPos.CreateTranslation(-etm.originalCenter);
				etm.moveFromZeroPos.CreateTranslation(etm.originalCenter);
				modifEntities.push_back(etm);

				// calc common for all selected entities bbox
				modifEntitiesBbox.AddAABBox(collisionSystem->GetBoundingBox(en));
			}
		}

		// now we has common bbox
		// center of this bbox will modification center, common for all entities
		modifEntitiesCenter = modifEntitiesBbox.GetCenter();

		// prepare translation matrix's, used before and after rotation
		moveToZeroPosRelativeCenter.CreateTranslation(-modifEntitiesCenter);
		moveFromZeroPosRelativeCenter.CreateTranslation(modifEntitiesCenter);

		// remember axis vector we are rotating around
		switch(curAxis)
		{
		case EM_AXIS_X:
		case EM_AXIS_XY:
			rotateAround = DAVA::Vector3(1, 0, 0);
			break;
		case EM_AXIS_YZ:
		case EM_AXIS_Y:
			rotateAround = DAVA::Vector3(0, 1, 0);
			break;
		case EM_AXIS_XZ:
		case EM_AXIS_Z:
			rotateAround = DAVA::Vector3(0, 0, 1);
			break;
		}

		// 2d axis projection we are rotating around
		DAVA::Vector2 rotateAxis = Cam2dProjection(modifEntitiesCenter, modifEntitiesCenter + rotateAround);

		// real rotate should be done in direction of 2dAxis normal,
		// so calculate this normal
		rotateNormal = DAVA::Vector2(-rotateAxis.y, rotateAxis.x);
		rotateNormal.Normalize();

		// remember current selection pivot point
		SceneSelectionSystem *selectionSystem = ((SceneEditorProxy *) GetScene())->selectionSystem;
		modifPivotPoint = selectionSystem->GetPivotPoint();
	}
}

void EntityModificationSystem::EndModification()
{
	modifEntitiesCenter.Set(0, 0, 0);
	modifEntities.clear();
	modifEntitiesBbox.Empty();
}

DAVA::Entity* EntityModificationSystem::GetFirstIntersectedEntity(const EntityGroup *gr1, const EntityGroup *gr2)
{
	DAVA::Entity* ret = NULL;

	for(size_t i = 0; i < gr1->Size(); ++i)
	{
		if(gr2->HasEntity(gr1->Get(i)))
		{
			ret = gr1->Get(i);
			break;
		}
	}

	return ret;
}

DAVA::Vector3 EntityModificationSystem::CamCursorPosToModifPos(const DAVA::Vector3 &camPosition, const DAVA::Vector3 &camPointDirection, const DAVA::Vector3 &planePoint)
{
	DAVA::Vector3 ret;
	DAVA::Vector3 planeNormal;

	switch(curAxis)
	{
	case EM_AXIS_X:
	case EM_AXIS_Y:
	case EM_AXIS_XY:
		planeNormal = DAVA::Vector3(0, 0, 1);
		break;
	case EM_AXIS_Z:
	case EM_AXIS_YZ:
		planeNormal = DAVA::Vector3(1, 0, 0);
		break;
	case EM_AXIS_XZ:
		planeNormal = DAVA::Vector3(0, 1, 0);
		break;
	default:
		break;
	}

	DAVA::GetIntersectionVectorWithPlane(camPosition, camPointDirection, planeNormal, planePoint, ret);
	return ret;
}

DAVA::Vector2 EntityModificationSystem::Cam2dProjection(const DAVA::Vector3 &from, const DAVA::Vector3 &to)
{
	DAVA::Vector2 axisBegin = cameraSystem->GetSñreenPos(from);
	DAVA::Vector2 axisEnd = cameraSystem->GetSñreenPos(to);
	DAVA::Vector2 ret = axisEnd - axisBegin;
	ret.Normalize();

	return ret;
}

void EntityModificationSystem::Move(const DAVA::Vector3 &newPos3d)
{
	for (size_t i = 0; i < modifEntities.size(); ++i)
	{
		DAVA::Vector3 modifPosWithLocedAxis = newPos3d;

		switch(curAxis)
		{
		case EM_AXIS_X:
			modifPosWithLocedAxis.y = modifStartPos3d.y;
			modifPosWithLocedAxis.z = modifStartPos3d.z;
			break;
		case EM_AXIS_Y:
			modifPosWithLocedAxis.x = modifStartPos3d.x;
			modifPosWithLocedAxis.z = modifStartPos3d.z;
			break;
		case EM_AXIS_Z:
			modifPosWithLocedAxis.x = modifStartPos3d.x;
			modifPosWithLocedAxis.y = modifStartPos3d.y;
			break;
		case EM_AXIS_XY:
			modifPosWithLocedAxis.z = modifStartPos3d.z;
			break;
		case EM_AXIS_XZ:
			modifPosWithLocedAxis.y = modifStartPos3d.y;
			break;
		case EM_AXIS_YZ:
			modifPosWithLocedAxis.x = modifStartPos3d.x;
			break;
		}

		DAVA::Vector3 moveOffset = modifPosWithLocedAxis - modifStartPos3d;
		DAVA::Matrix4 moveModification;
		moveModification.Identity();
		moveModification.CreateTranslation(moveOffset);

		modifEntities[i].entity->SetLocalTransform(modifEntities[i].originalTransform * moveModification);

		// also move hood
		hoodSystem->MovePosition(moveOffset);
	}
}

void EntityModificationSystem::Rotate(const DAVA::Vector2 &newPos2d)
{
	DAVA::Vector2 rotateLength = newPos2d - modifStartPos2d;
	DAVA::float32 rotateForce = -(rotateNormal.DotProduct(rotateLength));

	for (size_t i = 0; i < modifEntities.size(); ++i)
	{
		DAVA::Matrix4 rotateModification;
		rotateModification.Identity();
		rotateModification.CreateRotation(rotateAround, rotateForce / 70.0f);

		switch(modifPivotPoint)
		{
		case SceneSelectionSystem::SELECTION_ENTITY_CENTER:
			// move to zero, rotate, move back to original center point
			rotateModification = (modifEntities[i].moveToZeroPos * rotateModification) * modifEntities[i].moveFromZeroPos;
			break;
		case SceneSelectionSystem::SELECTION_COMMON_CENTER:
			// move to zero relative selection center, rotate, move back to original center point
			rotateModification = (moveToZeroPosRelativeCenter * rotateModification) * moveFromZeroPosRelativeCenter;
			break;
		default:
			rotateModification.Identity();
			break;
		}

		modifEntities[i].entity->SetLocalTransform(modifEntities[i].originalTransform * rotateModification);
	}
}

void EntityModificationSystem::Scale(const DAVA::Vector2 &newPos2d)
{

}
