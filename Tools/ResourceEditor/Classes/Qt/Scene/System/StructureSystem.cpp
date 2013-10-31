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



#include "Scene/System/StructureSystem.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/SceneSignals.h"
#include "Scene/SceneEditor2.h"

#include "Commands2/EntityParentChangeCommand.h"
#include "Commands2/EntityAddCommand.h"
#include "Commands2/EntityRemoveCommand.h"
#include "Commands2/ParticleLayerMoveCommand.h"
#include "Commands2/ParticleLayerRemoveCommand.h"
#include "Commands2/ParticleForceMoveCommand.h"
#include "Commands2/ParticleForceRemoveCommand.h"

#include "Classes/SceneEditor/SceneValidator.h"

#include "Scene3D/Components/ActionComponent.h"
#include "Scene3D/Components/SwitchComponent.h"

StructureSystem::StructureSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
	, structureChanged(false)
{
    new DAVA::SwitchComponent();
    new DAVA::ActionComponent();
}

StructureSystem::~StructureSystem()
{

}

bool StructureSystem::Init(const DAVA::FilePath & path)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL == sceneEditor)
	{
		return false;
	}

	Entity* entity = Load(path, false);
	if(NULL == entity)
	{
		return false;
	}

	DAVA::Vector<DAVA::Entity*> tmpEntities;
	int entitiesCount = entity->GetChildrenCount();

	// remember all child pointers, but don't add them to scene in this cycle
	// because when entity is adding it is automatically removing from its old hierarchy
	tmpEntities.reserve(entitiesCount);
	for (DAVA::int32 i = 0; i < entitiesCount; ++i)
	{
		tmpEntities.push_back(entity->GetChild(i));
	}

	// now we can safely add entities into our hierarchy
	for (DAVA::int32 i = 0; i < (DAVA::int32) tmpEntities.size(); ++i)
	{
		sceneEditor->AddNode(tmpEntities[i]);
	}

	entity->Release();
	return true;
}

void StructureSystem::Move(const EntityGroup &entityGroup, DAVA::Entity *newParent, DAVA::Entity *newBefore)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor && entityGroup.Size() > 0)
	{
		if(entityGroup.Size() > 1)
		{
			sceneEditor->BeginBatch("Move entities");
		}

		for(size_t i = 0; i < entityGroup.Size(); ++i)
		{
			sceneEditor->Exec(new EntityParentChangeCommand(entityGroup.GetEntity(i), newParent, newBefore));
		}

		if(entityGroup.Size() > 1)
		{
			sceneEditor->EndBatch();
		}

		EmitChanged();
	}
}

void StructureSystem::Remove(const EntityGroup &entityGroup)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor && entityGroup.Size() > 0)
	{
		if(entityGroup.Size() > 1)
		{
			sceneEditor->BeginBatch("Remove entities");
		}

		for(size_t i = 0; i < entityGroup.Size(); ++i)
		{
			sceneEditor->Exec(new EntityRemoveCommand(entityGroup.GetEntity(i)));
		}


		if(entityGroup.Size() > 1)
		{
			sceneEditor->EndBatch();
		}

		EmitChanged();
	}
}

void StructureSystem::MoveLayer(const DAVA::Vector<DAVA::ParticleLayer *> &layers, DAVA::ParticleEmitter *newEmitter, DAVA::ParticleLayer *newBefore)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		if(layers.size() > 1)
		{
			sceneEditor->BeginBatch("Move particle layers");
		}

		for(size_t i = 0; i < layers.size(); ++i)
		{
			sceneEditor->Exec(new ParticleLayerMoveCommand(layers[i], newEmitter, newBefore));
		}

		if(layers.size() > 1)
		{
			sceneEditor->EndBatch();
		}

		EmitChanged();
	}
}


void StructureSystem::RemoveLayer(const DAVA::Vector<DAVA::ParticleLayer *> &layers)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		if(layers.size() > 1)
		{
			sceneEditor->BeginBatch("Remove particle layers");
		}

		for(size_t i = 0; i < layers.size(); ++i)
		{
			sceneEditor->Exec(new ParticleLayerRemoveCommand(layers[i]));
		}

		if(layers.size() > 1)
		{
			sceneEditor->EndBatch();
		}

		EmitChanged();
	}
}

void StructureSystem::MoveForce(const DAVA::Vector<DAVA::ParticleForce *> &forces, const DAVA::Vector<DAVA::ParticleLayer *> &oldLayers, DAVA::ParticleLayer *newLayer)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		if(forces.size() > 1)
		{
			sceneEditor->BeginBatch("Move particle layers");
		}

		for(size_t i = 0; i < forces.size(); ++i)
		{
			sceneEditor->Exec(new ParticleForceMoveCommand(forces[i], oldLayers[i], newLayer));
		}

		if(forces.size() > 1)
		{
			sceneEditor->EndBatch();
		}

		EmitChanged();
	}
}

void StructureSystem::RemoveForce(const DAVA::Vector<DAVA::ParticleForce *> &forces, const DAVA::Vector<DAVA::ParticleLayer *> &layers)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		if(forces.size() > 1)
		{
			sceneEditor->BeginBatch("Remove particle layers");
		}

		for(size_t i = 0; i < forces.size(); ++i)
		{
			sceneEditor->Exec(new ParticleForceRemoveCommand(forces[i], layers[i]));
		}

		if(forces.size() > 1)
		{
			sceneEditor->EndBatch();
		}

		EmitChanged();
	}
}

void StructureSystem::Reload(const EntityGroup& entityGroup, const DAVA::FilePath &newModelPath, bool saveLightmapSettings)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor && entityGroup.Size() > 0)
	{
		DAVA::Vector<DAVA::Entity*> newEntities;
		SceneCollisionSystem *collisionSystem = sceneEditor->collisionSystem;

		bool loadSuccess = false;

		// load new models
		for(size_t i = 0; i < entityGroup.Size(); ++i)
		{
			DAVA::FilePath loadModelPath = newModelPath;
			DAVA::Entity *entity = entityGroup.GetEntity(i);

			newEntities.push_back(NULL);

			if(loadModelPath.IsEmpty())
			{
				DAVA::KeyedArchive *entityProperties = entity->GetCustomProperties();
				if(NULL != entityProperties)
				{
					loadModelPath = entityProperties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
				}
			}

			DAVA::Entity *loadedEntity = Load(loadModelPath, true);
			if(NULL != loadedEntity)
			{
				loadedEntity->GetCustomProperties()->SetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, loadModelPath.GetAbsolutePathname());
				newEntities[i] = loadedEntity;

				loadSuccess = true;
			}
		}

		// replace old models with new
		if(loadSuccess)
		{
			sceneEditor->BeginBatch("Reload model");
			for(size_t i = 0; i < entityGroup.Size(); ++i)
			{
				DAVA::Entity *origEntity = entityGroup.GetEntity(i);
				DAVA::Entity *newEntity = newEntities[i];

				if(NULL != origEntity && NULL != newEntity && NULL != origEntity->GetParent())
				{
					DAVA::Entity *before = origEntity->GetParent()->GetNextChild(origEntity);

					newEntity->SetLocalTransform(origEntity->GetLocalTransform());
                    
                    if(saveLightmapSettings)
                    {
                        CopyLightmapSettings(origEntity, newEntity);
                    }
					
					sceneEditor->Exec(new EntityParentChangeCommand(newEntity, origEntity->GetParent(), before));
					sceneEditor->Exec(new EntityRemoveCommand(origEntity));

					newEntity->Release();
				}
			}
			sceneEditor->EndBatch();

			// Перенести в Load и завалидейтить только подгруженную Entity
			// -->
            SceneValidator::Instance()->ValidateSceneAndShowErrors(sceneEditor, sceneEditor->GetScenePath());
			// <--

			EmitChanged();
		}
	}
}


void StructureSystem::Reload(const DAVA::FilePath &oldModelPath, const DAVA::FilePath &newModelPath, bool saveLightmapSettings)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		EntityGroup group;

		// reload only 1 level childs
		for (int i = 0; i < sceneEditor->GetChildrenCount(); i++)
		{
			DAVA::Entity *entity = sceneEditor->GetChild(i);
			if(NULL != entity->GetCustomProperties())
			{
				if(DAVA::FilePath(entity->GetCustomProperties()->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER)) == oldModelPath)
				{
					group.Add(entity);
				}
			}
		}

		Reload(group, newModelPath, saveLightmapSettings);
	}
}

void StructureSystem::Add(const DAVA::FilePath &newModelPath, const DAVA::Vector3 pos)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		DAVA::Entity *loadedEntity = Load(newModelPath, true);
		if(NULL != loadedEntity)
		{
			DAVA::Vector3 entityPos = pos;

			KeyedArchive *customProps = loadedEntity->GetCustomProperties();
            customProps->SetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, newModelPath.GetAbsolutePathname());

			if(entityPos.IsZero() && FindLandscape(loadedEntity) == NULL)
			{
				SceneCameraSystem *cameraSystem = sceneEditor->cameraSystem;

				DAVA::Vector3 camDirection = cameraSystem->GetCameraDirection();
				DAVA::Vector3 camPosition = cameraSystem->GetCameraPosition();

				DAVA::AABBox3 commonBBox = loadedEntity->GetWTMaximumBoundingBoxSlow();
				DAVA::float32 bboxSize = (commonBBox.max - commonBBox.min).Length();

				camDirection.Normalize();
				
				entityPos = camPosition + camDirection * bboxSize;
			}

			DAVA::Matrix4 transform = loadedEntity->GetLocalTransform();
			transform.SetTranslationVector(entityPos);
			loadedEntity->SetLocalTransform(transform);

            
			sceneEditor->Exec(new EntityAddCommand(loadedEntity, sceneEditor));

			// TODO: move this code to some another place (into command itself or into ProcessCommand function)
			// 
			// Перенести в Load и завалидейтить только подгруженную Entity
			// -->
			sceneEditor->UpdateShadowColorFromLandscape();
            SceneValidator::Instance()->ValidateSceneAndShowErrors(sceneEditor, sceneEditor->GetScenePath());
			// <--
            
			EmitChanged();
		}
	}
}

void StructureSystem::EmitChanged()
{
	// mark that structure was changed. real signal will be emited on next update() call
	// this should done be to increase perfomance - on Change emit on multiple scene structure operations
	structureChanged = true;
}

void StructureSystem::Update(DAVA::float32 timeElapsed)
{
	if(structureChanged)
	{
		SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), NULL);
		structureChanged = false;
	}
}

void StructureSystem::Draw()
{

}

void StructureSystem::ProcessUIEvent(DAVA::UIEvent *event)
{

}

void StructureSystem::ProcessCommand(const Command2 *command, bool redo)
{
	if(NULL != command)
	{
		int cmdId = command->GetId();
		if( cmdId == CMDID_PARTICLE_LAYER_REMOVE ||
			cmdId == CMDID_PARTICLE_LAYER_MOVE ||
			cmdId == CMDID_PARTICLE_FORCE_REMOVE ||
			cmdId == CMDID_PARTICLE_FORCE_MOVE)
		{
			EmitChanged();
		}
	}
}

void StructureSystem::AddEntity(DAVA::Entity * entity)
{
	DAVA::Entity *parent = (NULL != entity) ? entity->GetParent() : NULL;
	EmitChanged();
}

void StructureSystem::RemoveEntity(DAVA::Entity * entity)
{
	DAVA::Entity *parent = (NULL != entity) ? entity->GetParent() : NULL;
	EmitChanged();
}

void StructureSystem::CheckAndMarkSolid(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		if(entity->GetChildrenCount() > 0)
		{
			entity->SetSolid(true);

			for(DAVA::int32 i = 0; i < entity->GetChildrenCount(); ++i)
			{
				CheckAndMarkSolid(entity->GetChild(i));
			}
		}
		else
		{
			entity->SetSolid(false);
		}
	}
}

void StructureSystem::CheckAndMarkLocked(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		// mark lod childs as locked
		if(NULL != entity->GetComponent(DAVA::Component::LOD_COMPONENT))
		{
			for(int i = 0; i < entity->GetChildrenCount(); ++i)
			{
				MarkLocked(entity->GetChild(i));
			}
		}
		else
		{
			for(int i = 0; i < entity->GetChildrenCount(); ++i)
			{
				CheckAndMarkLocked(entity->GetChild(i));
			}
		}
	}
}

void StructureSystem::MarkLocked(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		entity->SetLocked(true);

		for(int i = 0; i < entity->GetChildrenCount(); ++i)
		{
			MarkLocked(entity->GetChild(i));
		}
	}
}

void SearchByReference(DAVA::Entity *entity, const DAVA::FilePath &ref, EntityGroup &result)
{
	if(NULL != entity)
	{
		DAVA::KeyedArchive *arch = entity->GetCustomProperties();
		if(DAVA::FilePath(arch->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, "")) == ref)
		{
			result.Add(entity);
		}

		for(int i = 0; i < entity->GetChildrenCount(); i++)
		{
			SearchByReference(entity->GetChild(i), ref, result);
		}
	}
}

DAVA::Entity* StructureSystem::Load(const DAVA::FilePath& sc2path, bool optimize)
{
	DAVA::Entity* loadedEntity = NULL;

	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor && sc2path.IsEqualToExtension(".sc2") && sc2path.Exists())
	{
		// if there is already entity for such file, we should release it
		// to be sure that latest version will be loaded 
        sceneEditor->ReleaseRootNode(sc2path);

		// load entity from file
		Entity *rootNode = sceneEditor->GetRootNode(sc2path);
        if(rootNode)
        {
            Entity *parentForOptimize = new Entity();
			parentForOptimize->AddNode(rootNode);

			if(optimize)
			{
				SceneFileV2 sceneFile;
				sceneFile.OptimizeScene(parentForOptimize);
			}

			if(parentForOptimize->GetChildrenCount())
			{
				loadedEntity = parentForOptimize->GetChild(0);
				loadedEntity->SetSolid(true);
				loadedEntity->Retain();
                
                CheckAndMarkLocked(loadedEntity);
                CheckAndMarkSolid(loadedEntity);
			}

			SafeRelease(parentForOptimize);

			// release loaded entity
			sceneEditor->ReleaseRootNode(sc2path);
			rootNode = NULL;
		}
	}

	return loadedEntity;
}

bool StructureSystem::CopyLightmapSettings(DAVA::Entity *fromEntity, DAVA::Entity *toEntity) const
{
    DAVA::Vector<DAVA::RenderObject *> fromMeshes;
    FindMeshesRecursive(fromEntity, fromMeshes);

    DAVA::Vector<DAVA::RenderObject *> toMeshes;
    FindMeshesRecursive(toEntity, toMeshes);
    
    if(fromMeshes.size() == toMeshes.size())
    {
        DAVA::uint32 meshCount = (DAVA::uint32)fromMeshes.size();
        for(DAVA::uint32 m = 0; m < meshCount; ++m)
        {
            DAVA::uint32 rbFromCount = fromMeshes[m]->GetRenderBatchCount();
            DAVA::uint32 rbToCount = toMeshes[m]->GetRenderBatchCount();
            
            if(rbFromCount != rbToCount)
                return false;
            
            for(DAVA::uint32 rb = 0; rb < rbFromCount; ++rb)
            {
                DAVA::RenderBatch *fromBatch = fromMeshes[m]->GetRenderBatch(rb);
                DAVA::RenderBatch *toBatch = toMeshes[m]->GetRenderBatch(rb);
                
                DAVA::InstanceMaterialState *fromState = fromBatch->GetMaterialInstance();
                DAVA::InstanceMaterialState *toState = toBatch->GetMaterialInstance();
                
                if(fromState && toState)
                {
                    toState->InitFromState(fromState);
                }
                
            }
        }
        
        return true;
    }

    return false;
}

void StructureSystem::FindMeshesRecursive(DAVA::Entity *entity, DAVA::Vector<DAVA::RenderObject *> & objects) const
{
    RenderObject *ro = GetRenderObject(entity);
    if(ro && ro->GetType() == RenderObject::TYPE_MESH)
    {
        objects.push_back(ro);
    }
    
	DAVA::int32 count = entity->GetChildrenCount();
	for(DAVA::int32 i = 0; i < count; ++i)
	{
        FindMeshesRecursive(entity->GetChild(i), objects);
	}
}


