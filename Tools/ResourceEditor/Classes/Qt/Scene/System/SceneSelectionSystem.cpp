#include <QApplication>

#include "Scene/System/SceneSelectionSystem.h"

#include "Scene/SceneEditorProxy.h"
#include "Scene/System/SceneCameraSystem.h"
#include "Scene/System/SceneCollisionSystem.h"

SceneSelectionSystem::SceneSelectionSystem(DAVA::Scene * scene, SceneCollisionSystem *collisionSystem)
	: DAVA::SceneSystem(scene)
	, collisionSystem(collisionSystem)
	, selectionDrawFlags(SELECTION_DRAW_SHAPE | SELECTION_FILL_SHAPE)
	, applyOnPhaseEnd(false)
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
		if(event->tid == DAVA::UIEvent::BUTTON_1)
		{
			DAVA::Entity *selectionEntity = NULL;
			SceneCameraSystem *cameraSystem	= ((SceneEditorProxy *) GetScene())->cameraSystem;

			DAVA::Vector3 camPos = cameraSystem->GetCameraPosition();
			DAVA::Vector3 camDir = cameraSystem->GetPointDirection(event->point);

			DAVA::Vector3 traceFrom = camPos;
			DAVA::Vector3 traceTo = traceFrom + camDir * 1000.0f;

			DAVA::Entity *selectAfterThat = NULL;
			if(curSelections.Size() > 0)
			{
				selectAfterThat = curSelections.Get(0);
			}

			const EntityGroup* collisionEntities = collisionSystem->RayTest(traceFrom, traceTo);
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
				if(selectionEntity == NULL || curSelections.HasEntity(selectionEntity))
				{
					applyOnPhaseEnd = true;
				}
				else
				{
					SetSelection(selectionEntity);
				}
			}

			lastSelection = selectionEntity;
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
}

void SceneSelectionSystem::AddSelection(DAVA::Entity *entity)
{
	curSelections.Add(entity, collisionSystem->GetBoundingBox(entity));
}

void SceneSelectionSystem::RemSelection(DAVA::Entity *entity)
{
	curSelections.Rem(entity);
}

const EntityGroup*  SceneSelectionSystem::GetSelection() const
{
	return &curSelections;
}
