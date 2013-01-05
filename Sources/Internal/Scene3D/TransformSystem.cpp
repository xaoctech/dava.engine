#include "Scene3D/TransformSystem.h"
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
	passedNodes = 0;
	multipliedNodes = 0;

	uint32 size = updatableEntities.size();
	for(uint32 i = 0; i < size; ++i)
	{
		HierahicFindUpdatableTransform(updatableEntities[i]);
	}
	updatableEntities.clear();

	if(passedNodes)
	{
		Logger::Info("TransformSystem %d passed %d multiplied", passedNodes, multipliedNodes);
	}
}

void TransformSystem::HierahicFindUpdatableTransform(SceneNode * entity)
{
	passedNodes++;

	if(entity->GetFlags() & SceneNode::TRANSFORM_NEED_UPDATE)
	{
		multipliedNodes++;
		TransformComponent * transform = entity->GetTransformComponent();
		if(transform->parentMatrix)
		{
			transform->worldMatrix = transform->localMatrix * *(transform->parentMatrix);
		}
	}

	entity->RemoveFlag(SceneNode::TRANSFORM_NEED_UPDATE);
	entity->RemoveFlag(SceneNode::TRANSFORM_DIRTY);

	uint32 size = entity->GetChildrenCount();
	for(uint32 i = 0; i < size; ++i)
	{
		HierahicFindUpdatableTransform(entity->GetChild(i));
	}
}

void TransformSystem::SortAndThreadSplit()
{
}

void TransformSystem::ImmediateUpdate(SceneNode * entity)
{
	HierahicNeedUpdate(entity);
	HierahicAddToUpdate(entity);
}

void TransformSystem::HierahicNeedUpdate(SceneNode * entity)
{
	if(!(entity->GetFlags() & SceneNode::TRANSFORM_NEED_UPDATE))
	{
		entity->AddFlag(SceneNode::TRANSFORM_NEED_UPDATE);
		uint32 size = entity->GetChildrenCount();
		for(uint32 i = 0; i < size; ++i)
		{
			HierahicNeedUpdate(entity->GetChild(i));
		}
	}
}

void TransformSystem::HierahicAddToUpdate(SceneNode * entity)
{
	if(!(entity->GetFlags() & SceneNode::TRANSFORM_DIRTY))
	{
		entity->AddFlag(SceneNode::TRANSFORM_DIRTY);
		SceneNode * parent = entity->GetParent();
		if(parent && parent->GetParent())
		{
			HierahicAddToUpdate(entity->GetParent());
		}
		else
		{//topmost parent
			updatableEntities.push_back(entity);
		}
	}
}
};
