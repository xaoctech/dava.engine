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
#include "Scene/System/CollisionSystem.h"
#include "Classes/SceneEditor/EditorConfig.h"

using namespace DAVA;

DebugDrawSystem::DebugDrawSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
	, objectType(ResourceEditor::ESOT_NONE)
    , objectTypeColor(Color::White())
{
    
}


DebugDrawSystem::~DebugDrawSystem()
{
	drawEntities.clear();
}


void DebugDrawSystem::SetRequestedObjectType( ResourceEditor::eSceneObjectType _objectType )
{
	drawEntities.clear();

	objectType = _objectType;

	if(ResourceEditor::ESOT_NONE != objectType)
	{
		EnumerateEntitiesForDrawRecursive(GetScene());
        
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
	DrawObjectBoxesByType();
}

void DebugDrawSystem::DrawObjectBoxesByType()
{
	SceneEditor2 *sc = (SceneEditor2 *)GetScene();
	SceneCollisionSystem *collSystem = sc->collisionSystem;
	SceneSelectionSystem *selSystem = sc->selectionSystem;

	if(!collSystem || !selSystem) return;

	int oldState = DAVA::RenderManager::Instance()->GetState();
	DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_TEST);

	DAVA::RenderManager::Instance()->SetColor(objectTypeColor);

	auto endIt = drawEntities.end();
	for(auto it = drawEntities.begin(); it != endIt; ++it)
	{
		AABBox3 worldBox = selSystem->GetSelectionAABox(*it, (*it)->GetWorldTransform());
		DAVA::RenderHelper::Instance()->DrawBox(worldBox);
	}

	DAVA::RenderManager::Instance()->ResetColor();
	DAVA::RenderManager::Instance()->SetState(oldState);
}

void DebugDrawSystem::EnumerateEntitiesForDrawRecursive( DAVA::Entity *entity )
{
 	KeyedArchive * customProperties = entity->GetCustomProperties();
 	if(customProperties && customProperties->IsKeyExists("CollisionType") && (customProperties->GetInt32("CollisionType", 0) == objectType))
 	{
 		drawEntities.push_back(entity);
 	}
 
 	uint32 count = entity->GetChildrenCount();
 	for(uint32 i = 0; i < count; ++i)
 	{
 		EnumerateEntitiesForDrawRecursive(entity->GetChild(i));
 	}
}



