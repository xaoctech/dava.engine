#include "Scene/System/EntityModifSystem.h"

#include "Scene/SceneEditorProxy.h"
#include "Scene/System/SceneCameraSystem.h"
#include "Scene/System/SceneCollisionSystem.h"
#include "Scene/System/SceneSelectionSystem.h"

EntityModificationSystem::EntityModificationSystem(DAVA::Scene * scene, SceneCollisionSystem *collisionSystem)
	: DAVA::SceneSystem(scene)
	, sceneCollisionSystem(collisionSystem)
	, modifState(false)
{

}

EntityModificationSystem::~EntityModificationSystem()
{

}

void EntityModificationSystem::Update(DAVA::float32 timeElapsed)
{

}

void EntityModificationSystem::ProcessUIEvent( DAVA::UIEvent *event )
{
	printf("%g, %g\n", event->point.x, event->point.y);

	if(NULL != sceneCollisionSystem)
	{
		const DAVA::Entity* foundEntity = NULL;

		// current selected entities
		SceneSelectionSystem *selectionSystem = ((SceneEditorProxy *) GetScene())->sceneSelectionSystem;
		const DAVA::Vector<DAVA::Entity*> *selectedEntities = selectionSystem->GetSelectionsAll();

		// if we are not in modification state, try to find some selected item
		// that have mouse cursor at the top of it
		if(!modifState)
		{
			SceneCameraSystem *cameraSystem	= ((SceneEditorProxy *) GetScene())->sceneCameraSystem;

			DAVA::Vector3 camPos = cameraSystem->GetCameraPosition();
			DAVA::Vector3 camDir = cameraSystem->GetPointDirection(event->point);

			DAVA::Vector3 traceFrom = camPos;
			DAVA::Vector3 traceTo = traceFrom + camDir * 1000.0f;

			// get intersected items in the line from camera to current mouse position
			const DAVA::Vector<DAVA::Entity*> *intersectedEntities = sceneCollisionSystem->RayTest(traceFrom, traceTo);

			// check if one of intersected item is selected
			for(size_t i = 0; i < intersectedEntities->size(); ++i)
			{
				for (size_t j = 0; j < selectedEntities->size(); j++)
				{
					if(intersectedEntities->operator[](i) == selectedEntities->operator[](j))
					{
						foundEntity = intersectedEntities->operator[](i);
						break;
					}
				}

				if(NULL != foundEntity)
				{
					break;
				}
			}
		}

		// we found entity, that is selected and has mouse cursor on the top of it
		if(NULL != foundEntity)
		{
			if(DAVA::UIEvent::PHASE_BEGAN == event->phase)
			{
				if(event->tid == DAVA::UIEvent::BUTTON_1)
				{
					// go to modification state
					modifState = true;
					modifStartPoint = event->point;
				}
			}
		}
		// or we are already in modification state
		else if(modifState)
		{
			if(event->phase == DAVA::UIEvent::PHASE_DRAG)
			{
				// modify current selected items
				for (size_t i = 0; i < selectedEntities->size(); i++)
				{
					DAVA::Entity *en = selectedEntities->operator[](i);

					DAVA::Matrix4 localTr = en->GetLocalTransform();
					DAVA::Matrix4 mofifTr(
						1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 0,
						0, 0, 0.15f, 0);

					en->SetLocalTransform(localTr * mofifTr);
				}
			}
			else if(event)
			{
				if(event->tid == DAVA::UIEvent::BUTTON_1)
				{
					modifState = false;
				}
			}
		}
	}
}

void EntityModificationSystem::Draw()
{

}
