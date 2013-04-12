#include "Scene/System/SelectionSystem.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/System/ModifSystem.h"

#include <QApplication>
#include "Scene/SceneEditorProxy.h"

SceneSelectionSystem::SceneSelectionSystem(DAVA::Scene * scene, SceneCollisionSystem *collSys, HoodSystem *hoodSys)
	: DAVA::SceneSystem(scene)
	, collisionSystem(collSys)
	, hoodSystem(hoodSys)
	, selectionDrawFlags(SELECTION_DRAW_SHAPE | SELECTION_FILL_SHAPE)
	, applyOnPhaseEnd(false)
	, curPivotPoint(SELECTION_COMMON_CENTER)
{

}

SceneSelectionSystem::~SceneSelectionSystem()
{

}

void SceneSelectionSystem::Update(DAVA::float32 timeElapsed)
{

}

void SceneSelectionSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	if(DAVA::UIEvent::PHASE_BEGAN == event->phase)
	{
		// we can select only if mouse isn't over hood axis
		if(EM_AXIS_NONE == hoodSystem->GetPassingAxis())
		{
			if(event->tid == DAVA::UIEvent::BUTTON_1)
			{
				DAVA::Entity *selectionEntity = NULL;
				DAVA::Entity *selectAfterThat = NULL;

				if(curSelections.Size() > 0)
				{
					selectAfterThat = curSelections.Get(0);
				}

				const EntityGroup* collisionEntities = collisionSystem->RayTestFromCamera();
				if(collisionEntities->Size() > 0)
				{
					// TODO:
					// search propriate selection
			
					/*
					btCollisionObject *btObj;

					// try to find object next after afterThatEntity
					DAVA::Entity *stepEntity = NULL;
					for(int i = 0; i < foundCount; ++i)
					{
						btCollisionObject *btObj = btCallback.m_collisionObjects[i];
						stepEntity = collisionToEntity.value(btObj, NULL);

						if(stepEntity == afterThatEntity && (i + 1) < foundCount)
						{
							retEntity = collisionToEntity.value(btCallback.m_collisionObjects[i + 1], NULL);
							break;
						}
					}

					// if not found - return first one
					if(NULL == retEntity)
					{
						btObj = btCallback.m_collisionObjects[0];
						retEntity = collisionToEntity.value(btObj, NULL);
					}
					*/

					selectionEntity = collisionEntities->Get(0);
				}

				int curKeyModifiers = QApplication::keyboardModifiers();
				if(curKeyModifiers & Qt::ControlModifier)
				{
					AddSelection(selectionEntity);
				}
				else if(curKeyModifiers & Qt::AltModifier)
				{
					RemSelection(selectionEntity);
				}
				else
				{
					// if new selection is NULL or is one of already selected items
					// we should change current selection only on phase end
					if(selectionEntity == NULL || curSelections.HasEntity(selectionEntity))
					{
						applyOnPhaseEnd = true;
						lastSelection = selectionEntity;
					}
					else
					{
						SetSelection(selectionEntity);
					}
				}
			}
		}
	}
	else if(DAVA::UIEvent::PHASE_ENDED == event->phase)
	{
		if(event->tid == DAVA::UIEvent::BUTTON_1)
		{
			if(applyOnPhaseEnd)
			{
				applyOnPhaseEnd = false;
				SetSelection(lastSelection);
			}
		}
	}
}

void SceneSelectionSystem::Draw()
{
	if(curSelections.Size() > 0)
	{
		int oldState = DAVA::RenderManager::Instance()->GetState();
		DAVA::eBlendMode oldBlendSrc = DAVA::RenderManager::Instance()->GetSrcBlend();
		DAVA::eBlendMode oldBlendDst = DAVA::RenderManager::Instance()->GetDestBlend();

		int newState = DAVA::RenderState::STATE_BLEND | DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE;
		if(!(selectionDrawFlags & SELECTION_NO_DEEP_TEST))
		{
			newState |= DAVA::RenderState::STATE_DEPTH_TEST;
		}

		DAVA::RenderManager::Instance()->SetState(newState);
		DAVA::RenderManager::Instance()->SetBlendMode(DAVA::BLEND_SRC_ALPHA, DAVA::BLEND_ONE_MINUS_SRC_ALPHA);

		for (DAVA::uint32 i = 0; i < curSelections.Size(); i++)
		{
			DAVA::AABBox3 selectionBox = collisionSystem->GetBoundingBox(curSelections.Get(i));

			// draw selection share
			if(selectionDrawFlags & SELECTION_DRAW_SHAPE)
			{
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f));
				DAVA::RenderHelper::Instance()->DrawBox(selectionBox);
			}

			// fill selection shape
			if(selectionDrawFlags & SELECTION_FILL_SHAPE)
			{
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 0.15f));
				DAVA::RenderHelper::Instance()->FillBox(selectionBox);
			}
		}

		DAVA::RenderManager::Instance()->SetBlendMode(oldBlendSrc, oldBlendDst);
		DAVA::RenderManager::Instance()->SetState(oldState);
	}
}

void SceneSelectionSystem::SetSelection(DAVA::Entity *entity)
{
	curSelections.Clear();
	curSelections.Add(entity, collisionSystem->GetBoundingBox(entity));

	UpdateHoodPos();
}

void SceneSelectionSystem::AddSelection(DAVA::Entity *entity)
{
	curSelections.Add(entity, collisionSystem->GetBoundingBox(entity));

	UpdateHoodPos();
}

void SceneSelectionSystem::RemSelection(DAVA::Entity *entity)
{
	curSelections.Rem(entity);

	UpdateHoodPos();
}

const EntityGroup*  SceneSelectionSystem::GetSelection() const
{
	return &curSelections;
}

void SceneSelectionSystem::SelectedItemsWereModified()
{
	// don't change selection on phase end
	applyOnPhaseEnd = false;
}

void SceneSelectionSystem::SetPivotPoint(int pp)
{
	curPivotPoint = pp;
}

int SceneSelectionSystem::GetPivotPoint() const
{
	return curPivotPoint;
}

void SceneSelectionSystem::UpdateHoodPos() const
{
	if(curSelections.Size() > 0)
	{
		DAVA::Vector3 p;
		switch (curPivotPoint)
		{
		case SELECTION_ENTITY_CENTER:
			p = curSelections.GetBbox(0).GetCenter();
			break;
		default:
			p = curSelections.GetCommonBbox().GetCenter();
			break;
		}

		hoodSystem->SetPosition(p);
		hoodSystem->Show();
	}
	else
	{
		hoodSystem->SetPosition(DAVA::Vector3(0, 0,0 ));
		hoodSystem->Hide();
	}
}
