#include "Scene3D/Components/TransformSystem.h"
#include "Scene3D/Components/Transform.h"
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

Transform * TransformSystem::CreateTransform()
{
    Transform * transform = new Transform();
	transform->localMatrix = Matrix4::IDENTITY;
	transform->worldMatrix = Matrix4::IDENTITY;
	transform->parentMatrix = 0;
	transform->parent = 0;

	return transform;
}

Transform * TransformSystem::CloneTransform(Transform * oldTransform)
{
	Transform * newTransform = CreateTransform();
	newTransform->localMatrix = oldTransform->localMatrix;
	newTransform->worldMatrix = oldTransform->worldMatrix;
	newTransform->parent = oldTransform->parent;

	return newTransform;
}

Transform * TransformSystem::GetTransformWithIncrement(Transform * transform)
{
	return transform;
}
    
void TransformSystem::DeleteTransform(Transform * deletingTransform)
{
	SafeDelete(deletingTransform);
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
		Transform * transform = updatableEntities[i]->GetTransform();
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

