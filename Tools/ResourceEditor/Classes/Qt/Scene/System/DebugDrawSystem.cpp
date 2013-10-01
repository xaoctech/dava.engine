/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "Scene/System/DebugDrawSystem.h"
#include "Scene/SceneEditor2.h"
#include "Classes/SceneEditor/EditorConfig.h"

using namespace DAVA;

DebugDrawSystem::DebugDrawSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
	, objectType(ResourceEditor::ESOT_NONE)
    , objectTypeColor(Color::White())
{
	SceneEditor2 *sc = (SceneEditor2 *)GetScene();

	collSystem = sc->collisionSystem;
	selSystem = sc->selectionSystem;

	DVASSERT(NULL != collSystem);
	DVASSERT(NULL != selSystem);
}


DebugDrawSystem::~DebugDrawSystem()
{ }

void DebugDrawSystem::SetRequestedObjectType(ResourceEditor::eSceneObjectType _objectType)
{
	objectType = _objectType;

	if(ResourceEditor::ESOT_NONE != objectType)
	{
        const Vector<Color> & colors = EditorConfig::Instance()->GetColorPropertyValues("CollisionTypeColor");
        if(objectType < colors.size())
        {
            objectTypeColor = colors[objectType];
        }
        else
        {
            objectTypeColor = Color(1.f, 0, 0, 1.f);
        }
	}
}


ResourceEditor::eSceneObjectType DebugDrawSystem::GetRequestedObjectType() const
{
	return objectType;
}

void DebugDrawSystem::Draw()
{
	int oldState = DAVA::RenderManager::Instance()->GetState();
	DAVA::eBlendMode oldBlendSrc = DAVA::RenderManager::Instance()->GetSrcBlend();
	DAVA::eBlendMode oldBlendDst = DAVA::RenderManager::Instance()->GetDestBlend();
	DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_BLEND | DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_TEST);
	DAVA::RenderManager::Instance()->SetBlendMode(DAVA::BLEND_SRC_ALPHA, DAVA::BLEND_ONE_MINUS_SRC_ALPHA);

	Draw(GetScene());

	DAVA::RenderManager::Instance()->SetBlendMode(oldBlendSrc, oldBlendDst);
	DAVA::RenderManager::Instance()->ResetColor();
	DAVA::RenderManager::Instance()->SetState(oldState);
}

void DebugDrawSystem::Draw(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		DrawObjectBoxesByType(entity);
		DrawUserNode(entity);
		DrawLightNode(entity);
		DrawSoundNode(entity);

		for(size_t i = 0; i < entity->GetChildrenCount(); ++i)
		{
			Draw(entity->GetChild(i));
		}
	}
}

inline void DebugDrawSystem::DrawObjectBoxesByType(DAVA::Entity *entity)
{
	KeyedArchive * customProperties = entity->GetCustomProperties();
	if(customProperties && customProperties->IsKeyExists("CollisionType") && (customProperties->GetInt32("CollisionType", 0) == objectType))
	{
		AABBox3 worldBox = selSystem->GetSelectionAABox(entity, entity->GetWorldTransform());

		DAVA::RenderManager::Instance()->SetColor(objectTypeColor);
		DAVA::RenderHelper::Instance()->DrawBox(worldBox);
	}
}

inline void DebugDrawSystem::DrawUserNode(DAVA::Entity *entity)
{
	if(NULL != entity->GetComponent(DAVA::Component::USER_COMPONENT))
	{
		Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
		Matrix4 finalMatrix = entity->GetWorldTransform() * prevMatrix;

		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);

		AABBox3 worldBox = selSystem->GetSelectionAABox(entity);
		DAVA::float32 delta = worldBox.GetSize().Length() / 4;

		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.5f, 0.5f, 1.0f, 0.3f));
		DAVA::RenderHelper::Instance()->FillBox(worldBox);
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.2f, 0.2f, 0.8f, 1.0f));
		DAVA::RenderHelper::Instance()->DrawBox(worldBox);

		// axises
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.7f, 0, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(delta, 0, 0));
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 0.7f, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, delta, 0));
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 0, 0.7f, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, 0, delta));

		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
	}
}

inline void DebugDrawSystem::DrawLightNode(DAVA::Entity *entity)
{
	if(NULL != entity->GetComponent(DAVA::Component::LIGHT_COMPONENT))
	{
		AABBox3 worldBox = selSystem->GetSelectionAABox(entity, entity->GetWorldTransform());

		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 0, 0.3f));
		DAVA::RenderHelper::Instance()->FillBox(worldBox);
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawBox(worldBox);
	}
}

inline void DebugDrawSystem::DrawSoundNode(DAVA::Entity *entity)
{
	if(NULL != entity->GetComponent(DAVA::Component::SOUND_COMPONENT))
	{
		AABBox3 worldBox = selSystem->GetSelectionAABox(entity, entity->GetWorldTransform());

		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 0.3f, 0.8f, 0.3f));
		DAVA::RenderHelper::Instance()->FillBox(worldBox);
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0, 0.3f, 0.8, 1.0f));
		DAVA::RenderHelper::Instance()->DrawBox(worldBox);
	}
}
