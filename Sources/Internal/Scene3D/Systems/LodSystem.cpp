#include "Scene3D/Systems/LodSystem.h"
#include "Debug/DVAssert.h"
#include "Scene3D/SceneNode.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Camera.h"

namespace DAVA
{

LodSystem::LodSystem()
{
	camera = 0;
	UpdatePartialUpdateIndices();
}

void LodSystem::Process()
{
	for(int32 i = partialUpdateIndices[currentPartialUpdateIndex]; i < partialUpdateIndices[currentPartialUpdateIndex+1]; ++i)
	{
		SceneNode * entity = entities[i];
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

void LodSystem::AddEntity(SceneNode * entity)
{
	entities.push_back(entity);
	UpdatePartialUpdateIndices();
}

void LodSystem::RemoveEntity(SceneNode * entity)
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

void LodSystem::UpdateEntityAfterLoad(SceneNode * entity)
{
	LodComponent * lod = static_cast<LodComponent*>(entity->GetComponent(Component::LOD_COMPONENT));
	for (List<LodComponent::LodData>::iterator it = lod->lodLayers.begin(); it != lod->lodLayers.end(); ++it)
	{
		LodComponent::LodData & ld = *it;
		size_t size = ld.indexes.size();
		for (size_t idx = 0; idx < size; ++idx)
		{
			SceneNode * childEntity = entity->GetChild(ld.indexes[idx]);
			ld.nodes.push_back(childEntity);
			{
				childEntity->SetUpdatable(false);
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

void LodSystem::UpdateLod(SceneNode * entity)
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
				oldLod->nodes[i]->SetUpdatable(false);
			}
		}
		int32 size = lodComponent->currentLod->nodes.size();
		for (int i = 0; i < size; i++) 
		{
			lodComponent->currentLod->nodes[i]->SetUpdatable(true);
		}
	}
}

void LodSystem::RecheckLod(SceneNode * entity)
{
	LodComponent * lodComponent = static_cast<LodComponent*>(entity->GetComponent(Component::LOD_COMPONENT));
	if (!lodComponent->currentLod)return;

	if(LodComponent::INVALID_LOD_LAYER != lodComponent->forceLodLayer) 
	{
		for (List<LodComponent::LodData>::iterator it = lodComponent->lodLayers.begin(); it != lodComponent->lodLayers.end(); it++)
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
			for (List<LodComponent::LodData>::iterator it = lodComponent->lodLayers.begin(); it != lodComponent->lodLayers.end(); it++)
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

}
