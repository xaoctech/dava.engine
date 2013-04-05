#include <QApplication>

#include "Scene/System/SceneSelectionSystem.h"

#include "Scene/SceneEditorProxy.h"
#include "Scene/System/SceneCameraSystem.h"
#include "Scene/System/SceneCollisionSystem.h"

SceneSelectionSystem::SceneSelectionSystem(DAVA::Scene * scene, SceneCollisionSystem *collisionSystem)
	: DAVA::SceneSystem(scene)
	, sceneCollisionSystem(collisionSystem)
	, selectionDrawFlags(SELECTION_DRAW_SHAPE | SELECTION_FILL_SHAPE)
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
			SceneCameraSystem *cameraSystem	= ((SceneEditorProxy *) GetScene())->sceneCameraSystem;

			DAVA::Vector3 camPos = cameraSystem->GetCameraPosition();
			DAVA::Vector3 camDir = cameraSystem->GetPointDirection(event->point);

			DAVA::Vector3 traceFrom = camPos;
			DAVA::Vector3 traceTo = traceFrom + camDir * 1000.0f;

			DAVA::Entity *selectAfterThat = NULL;
			if(curSelections.size() > 0)
			{
				selectAfterThat = curSelections[0];
			}

			const DAVA::Vector<DAVA::Entity*> *intersectedEntities = sceneCollisionSystem->RayTest(traceFrom, traceTo);

			DAVA::Entity *newSelection = NULL;

			if(intersectedEntities->size() > 0)
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

				newSelection = intersectedEntities->operator[](0);
			}


			// TODO:
			// check modifiers key and make add/remove to selection
			// ...
			int curKeyModifiers = QApplication::keyboardModifiers();
			if(curKeyModifiers & Qt::ShiftModifier)
			{
				AddSelection(newSelection);
			}
			else if(curKeyModifiers & Qt::AltModifier)
			{
				RemSelection(newSelection);
			}
			else
			{
				SetSelection(newSelection);
			}
		}
	}
}

void SceneSelectionSystem::Draw()
{
	if(curSelections.size() > 0)
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

		for (DAVA::uint32 i = 0; i < curSelections.size(); i++)
		{
			DAVA::AABBox3 selectionBox = sceneCollisionSystem->GetBoundingBox(curSelections[i]);

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
				DAVA::Vector3 min = selectionBox.min;
				DAVA::Vector3 max = selectionBox.max;

				DAVA::Polygon3 poly;
				poly.AddPoint(min);
				poly.AddPoint(DAVA::Vector3(min.x, min.y, max.z));
				poly.AddPoint(DAVA::Vector3(min.x, max.y, max.z));
				poly.AddPoint(DAVA::Vector3(min.x, max.y, min.z));
				DAVA::RenderHelper::Instance()->FillPolygon(poly);

				poly.Clear();
				poly.AddPoint(min);
				poly.AddPoint(DAVA::Vector3(min.x, max.y, min.z));
				poly.AddPoint(DAVA::Vector3(max.x, max.y, min.z));
				poly.AddPoint(DAVA::Vector3(max.x, min.y, min.z));
				DAVA::RenderHelper::Instance()->FillPolygon(poly);

				poly.Clear();
				poly.AddPoint(min);
				poly.AddPoint(DAVA::Vector3(min.x, min.y, max.z));
				poly.AddPoint(DAVA::Vector3(max.x, min.y, max.z));
				poly.AddPoint(DAVA::Vector3(max.x, min.y, min.z));
				DAVA::RenderHelper::Instance()->FillPolygon(poly);

				poly.Clear();
				poly.AddPoint(max);
				poly.AddPoint(DAVA::Vector3(max.x, max.y, min.z));
				poly.AddPoint(DAVA::Vector3(max.x, min.y, min.z));
				poly.AddPoint(DAVA::Vector3(max.x, min.y, max.z));
				DAVA::RenderHelper::Instance()->FillPolygon(poly);

				poly.Clear();
				poly.AddPoint(max);
				poly.AddPoint(DAVA::Vector3(max.x, max.y, min.z));
				poly.AddPoint(DAVA::Vector3(min.x, max.y, min.z));
				poly.AddPoint(DAVA::Vector3(min.x, max.y, max.z));
				DAVA::RenderHelper::Instance()->FillPolygon(poly);

				poly.Clear();
				poly.AddPoint(max);
				poly.AddPoint(DAVA::Vector3(max.x, min.y, max.z));
				poly.AddPoint(DAVA::Vector3(min.x, min.y, max.z));
				poly.AddPoint(DAVA::Vector3(min.x, max.y, max.z));
				DAVA::RenderHelper::Instance()->FillPolygon(poly);
			}
		}

		DAVA::RenderManager::Instance()->SetBlendMode(oldBlendSrc, oldBlendDst);
		DAVA::RenderManager::Instance()->SetState(oldState);
	}
}

void SceneSelectionSystem::SetSelection(DAVA::Entity *entity)
{
	curSelections.clear();
	curSelections.push_back(entity);
}


void SceneSelectionSystem::AddSelection(DAVA::Entity *entity)
{
	curSelections.push_back(entity);
}

void SceneSelectionSystem::RemSelection(DAVA::Entity *entity)
{
	// TODO:
	// ...
}

DAVA::Entity* SceneSelectionSystem::GetSelectionSingle()
{
	DAVA::Entity* singleSel = NULL;

	if(curSelections.size())
	{
		singleSel = curSelections[0];
	}

	return singleSel;
}

const DAVA::Vector<DAVA::Entity *>* SceneSelectionSystem::GetSelectionsAll()
{
	return &curSelections;
}
