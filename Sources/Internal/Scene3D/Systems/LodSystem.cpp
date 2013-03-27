#include "Scene3D/Systems/LodSystem.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/LodComponent.h"
#include "Render/Highlevel/Camera.h"
#include "Scene3D/Components/ComponentHelpers.h"

namespace DAVA
{

LodSystem::LodSystem(Scene * scene)
:	SceneSystem(scene)
{
	camera = 0;
	UpdatePartialUpdateIndices();
}

void LodSystem::Process()
{
	for(int32 i = partialUpdateIndices[currentPartialUpdateIndex]; i < partialUpdateIndices[currentPartialUpdateIndex+1]; ++i)
	{
		Entity * entity = entities[i];
		LodComponent * lod = static_cast<LodComponent*>(entity->GetComponent(Component::LOD_COMPONENT));
		if(lod->flags & LodComponent::NEED_UPDATE_AFTER_LOAD)
		{
			UpdateEntityAfterLoad(entity);
			lod->flags &= ~LodComponent::NEED_UPDATE_AFTER_LOAD;
		}

		UpdateLod(entity);
	}

	currentPartialUpdateIndex = currentPartialUpdateIndex < UPDATE_PART_PER_FRAME-1 ? currentPartialUpdateIndex+1 : 0;
}

void LodSystem::AddEntity(Entity * entity)
{
	entities.push_back(entity);
	UpdatePartialUpdateIndices();
}

void LodSystem::RemoveEntity(Entity * entity)
{
	uint32 size = entities.size();
	for(uint32 i = 0; i < size; ++i)
	{
		if(entities[i] == entity)
		{
			entities[i] = entities[size-1];
			entities.pop_back();
			UpdatePartialUpdateIndices();
			return;
		}
	}
	
	DVASSERT(0);
}

void LodSystem::UpdateEntityAfterLoad(Entity * entity)
{
	LodComponent * lod = static_cast<LodComponent*>(entity->GetComponent(Component::LOD_COMPONENT));
	for (Vector<LodComponent::LodData>::iterator it = lod->lodLayers.begin(); it != lod->lodLayers.end(); ++it)
	{
		LodComponent::LodData & ld = *it;
		size_t size = ld.indexes.size();
		for (size_t idx = 0; idx < size; ++idx)
		{
			Entity * childEntity = entity->GetChild(ld.indexes[idx]);
			ld.nodes.push_back(childEntity);
			{
				childEntity->SetLodVisible(false);
			}
		}
	}

	lod->currentLod = NULL;
	if(lod->lodLayers.size() > 0)
	{
		lod->SetCurrentLod(&(*lod->lodLayers.rbegin()));
	}
}

void LodSystem::UpdatePartialUpdateIndices()
{
	currentPartialUpdateIndex = 0;

	int32 size = (int32)entities.size();
	int32 partSize = size/UPDATE_PART_PER_FRAME;
	
	partialUpdateIndices.clear();
	int32 currentIndex = 0;
	partialUpdateIndices.push_back(currentIndex);
	for(int32 i = 0; i < UPDATE_PART_PER_FRAME; ++i)
	{
		currentIndex += partSize;
		partialUpdateIndices.push_back(currentIndex);
	}

	int32 & LastSlot = partialUpdateIndices[partialUpdateIndices.size()-1];
	LastSlot = Max(LastSlot, size);
}

void LodSystem::UpdateLod(Entity * entity)
{
	LodComponent * lodComponent = static_cast<LodComponent*>(entity->GetComponent(Component::LOD_COMPONENT));
	LodComponent::LodData * oldLod = lodComponent->currentLod;
	RecheckLod(entity);
	if (oldLod != lodComponent->currentLod) 
	{
		if (oldLod) 
		{
			int32 size = oldLod->nodes.size();
			for (int i = 0; i < size; i++) 
			{
				oldLod->nodes[i]->SetLodVisible(false);
			}
		}
		int32 size = lodComponent->currentLod->nodes.size();
		for (int i = 0; i < size; i++) 
		{
			lodComponent->currentLod->nodes[i]->SetLodVisible(true);
		}
	}
}

void LodSystem::RecheckLod(Entity * entity)
{
	LodComponent * lodComponent = static_cast<LodComponent*>(entity->GetComponent(Component::LOD_COMPONENT));
	if (!lodComponent->currentLod)return;

	if(LodComponent::INVALID_LOD_LAYER != lodComponent->forceLodLayer) 
	{
		for (Vector<LodComponent::LodData>::iterator it = lodComponent->lodLayers.begin(); it != lodComponent->lodLayers.end(); it++)
		{
			if (it->layer >= lodComponent->forceLodLayer)
			{
				lodComponent->currentLod = &(*it);
				return;
			}
		}
		return;
	}

	{
		float32 dst = 0.f;
		if(LodComponent::INVALID_DISTANCE == lodComponent->forceDistance)
		{
			if(camera)
			{
				dst = (camera->GetPosition() - entity->GetWorldTransform().GetTranslationVector()).SquareLength();
				dst *= camera->GetZoomFactor() * camera->GetZoomFactor();
			}
		}
		else 
		{
			dst = lodComponent->forceDistanceSq;
		}

		if (dst > lodComponent->GetLodLayerFarSquare(lodComponent->currentLod->layer) || dst < lodComponent->GetLodLayerNearSquare(lodComponent->currentLod->layer))
		{
			for (Vector<LodComponent::LodData>::iterator it = lodComponent->lodLayers.begin(); it != lodComponent->lodLayers.end(); it++)
			{
				if (dst >= lodComponent->GetLodLayerNearSquare(it->layer))
				{
					lodComponent->currentLod = &(*it);
				}
				else 
				{
					return;
				}
			}
		}
	}
}

void LodSystem::SetCamera(Camera * _camera)
{
	camera = _camera;
}

void LodSystem::MergeChildLods(Entity * toEntity)
{
	LodSystem::LodMerger merger(toEntity);
	merger.MergeChildLods();
}

LodSystem::LodMerger::LodMerger(Entity * _toEntity)
{
	DVASSERT(_toEntity);
	toEntity = _toEntity;
}

void LodSystem::LodMerger::MergeChildLods()
{
	LodComponent * toLod = (LodComponent*)toEntity->GetOrCreateComponent(Component::LOD_COMPONENT);
	

	Vector<Entity*> allLods;
	GetLodComponentsRecursive(toEntity, allLods);

	uint32 count = allLods.size();
	for(uint32 i = 0; i < count; ++i)
	{
		LodComponent * fromLod = GetLodComponent(allLods[i]);
		int32 fromLodsCount = fromLod->GetMaxLodLayer();
		for(int32 l = 0; l <= fromLodsCount; ++l)
		{
			LodComponent::LodData & fromData = fromLod->lodLayers[l];
			int32 lodLayerIndex = fromData.layer;

			LodComponent::LodData * toData = 0;

			int32 maxLod = toLod->GetMaxLodLayer();
			//create loddata if needed
			if(lodLayerIndex > maxLod)
			{
				DVASSERT(maxLod == lodLayerIndex-1);

				toLod->lodLayers.push_back(fromData);
				toData = &(toLod->lodLayers[lodLayerIndex]);
				toData->nodes.clear();
				toData->indexes.clear(); //indeces will not have any sense after lod merge

				toLod->lodLayersArray = fromLod->lodLayersArray;
			}
			else
			{
				toData = &(toLod->lodLayers[lodLayerIndex]);
			}

			uint32 nodesToCopy = fromData.nodes.size();
			for(uint32 j = 0; j < nodesToCopy; ++j)
			{
				toData->nodes.push_back(fromData.nodes[j]); 
			}
		}

		allLods[i]->RemoveComponent(Component::LOD_COMPONENT);
	}
}

void LodSystem::LodMerger::GetLodComponentsRecursive(Entity * fromEntity, Vector<Entity*> & allLods)
{
	if(fromEntity != toEntity)
	{
		LodComponent * lod = GetLodComponent(fromEntity);
		if(lod)
		{
			if(lod->flags & LodComponent::NEED_UPDATE_AFTER_LOAD)
			{
				LodSystem::UpdateEntityAfterLoad(fromEntity);
				lod->flags &= ~LodComponent::NEED_UPDATE_AFTER_LOAD;
			}

			allLods.push_back(fromEntity);
		}
	}
	int32 count = fromEntity->GetChildrenCount();
	for(int32 i = 0; i < count; ++i)
	{
		GetLodComponentsRecursive(fromEntity->GetChild(i), allLods);
	}
}

}
