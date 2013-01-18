#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/SceneNode.h"

namespace DAVA
{

const float32 LodComponent::INVALID_DISTANCE = -1.f;
const float32 LodComponent::MIN_LOD_DISTANCE = 0.f;
const float32 LodComponent::MAX_LOD_DISTANCE = 500.f;

LodComponent::LodDistance::LodDistance()
{
	distance = nearDistance = nearDistanceSq = farDistance = farDistanceSq = (float32) INVALID_DISTANCE;
}

void LodComponent::LodDistance::SetDistance(const float32 &newDistance)
{
	distance = newDistance;
}

void LodComponent::LodDistance::SetNearDistance(const float32 &newDistance)
{
	nearDistance = newDistance;
	nearDistanceSq = nearDistance * nearDistance;
}

void LodComponent::LodDistance::SetFarDistance(const float32 &newDistance)
{
	farDistance = newDistance;
	farDistanceSq = farDistance * farDistance;
}

Component * LodComponent::Clone()
{
	LodComponent * newLod = new LodComponent();
    // TODO: Add lod cloning
	return newLod;
}

LodComponent::LodComponent()
:	forceLodLayer(INVALID_LOD_LAYER),
	forceDistance(INVALID_DISTANCE),
	forceDistanceSq(INVALID_DISTANCE)
{
	flags = NEED_UPDATE_AFTER_LOAD;

	for(int32 iLayer = 0; iLayer < MAX_LOD_LAYERS; ++iLayer)
	{
		lodLayersArray[iLayer].SetDistance(GetDefaultDistance(iLayer));
		lodLayersArray[iLayer].SetFarDistance(MAX_LOD_DISTANCE * 2);
	}

	lodLayersArray[0].SetNearDistance(0.0f);
}

float32 LodComponent::GetDefaultDistance(int32 layer)
{
	float32 distance = MIN_LOD_DISTANCE + ((float32)(MAX_LOD_DISTANCE - MIN_LOD_DISTANCE) / (MAX_LOD_LAYERS-1)) * layer;
	return distance;
}

void LodComponent::SetCurrentLod(LodData *newLod)
{
	if (newLod != currentLod) 
	{
		if (currentLod) 
		{
			int32 size = currentLod->nodes.size();
			for (int i = 0; i < size; i++) 
			{
				currentLod->nodes[i]->SetUpdatable(false);
			}
		}
		currentLod = newLod;
		int32 size = currentLod->nodes.size();
		for (int i = 0; i < size; i++) 
		{
			currentLod->nodes[i]->SetUpdatable(true);
		}
	}
}

};
