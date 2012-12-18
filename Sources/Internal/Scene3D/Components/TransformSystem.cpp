#include "Scene3D/Components/TransformSystem.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/SceneNode.h"
#include "Debug/DVAssert.h"

namespace DAVA
{



TransformSystem::TransformSystem()
{
}

TransformSystem::~TransformSystem()
{
}

void TransformSystem::LinkTransform(int32 parentIndex, int32 childIndex)
{
}

void TransformSystem::UnlinkTransform(int32 childIndex)
{
}

void TransformSystem::Process()
{
	uint32 size = updatableEntities.size();
	for(uint32 i = 0; i < size; ++i)
	{
		TransformComponent * transform = updatableEntities[i]->GetTransformComponent();
		if(transform->parentMatrix)
		{
			transform->worldMatrix = transform->localMatrix * *(transform->parentMatrix);
		}
	}
	updatableEntities.clear();
}

void TransformSystem::SortAndThreadSplit()
{
}

void TransformSystem::NeedUpdate(SceneNode * entity)
{
	HierahicNeedUpdate(entity);
	
}

void TransformSystem::HierahicNeedUpdate(SceneNode * entity)
{
	updatableEntities.push_back(entity);
	uint32 size = entity->GetChildrenCount();
	for(uint32 i = 0; i < size; ++i)
	{
		HierahicNeedUpdate(entity->GetChild(i));
	}
}

};

