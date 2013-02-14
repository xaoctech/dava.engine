#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/SceneNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{
    
TransformComponent::TransformComponent()
{
    localMatrix = Matrix4::IDENTITY;
	worldMatrix = Matrix4::IDENTITY;
	parentMatrix = 0;
	parent = 0;

	GlobalEventSystem::Instance()->Event(0, this, EventSystem::LOCAL_TRANSFORM_CHANGED);
}
    
TransformComponent::~TransformComponent()
{
    
}

Component * TransformComponent::Clone(SceneNode * toEntity)
{
    TransformComponent * newTransform = new TransformComponent();
	newTransform->SetEntity(toEntity);
	newTransform->localMatrix = localMatrix;
	newTransform->worldMatrix = worldMatrix;
    newTransform->parent = this->parent;

    return newTransform;
}


void TransformComponent::SetLocalTransform(const Matrix4 * transform)
{
	localMatrix = *transform;
	if(!parent)
	{
		worldMatrix = *transform;
	}

	GlobalEventSystem::Instance()->Event(entity, this, EventSystem::LOCAL_TRANSFORM_CHANGED);
}

void TransformComponent::SetParent(SceneNode * node)
{
	parent = node;

	if(node)
	{
		parentMatrix = ((TransformComponent*)node->GetComponent(Component::TRANSFORM_COMPONENT))->GetWorldTransformPtr();
	}
	else
	{
		parentMatrix = 0;
	}

	GlobalEventSystem::Instance()->Event(entity, this, EventSystem::TRANSFORM_PARENT_CHANGED);
}

Matrix4 & TransformComponent::ModifyLocalTransform()
{
	GlobalEventSystem::Instance()->Event(entity, this, EventSystem::LOCAL_TRANSFORM_CHANGED);
	return localMatrix;
}

void TransformComponent::Serialize(KeyedArchive *archive)
{
	Component::Serialize(archive);

	if(NULL != archive)
	{
		archive->SetMatrix4("tc.localMatrix", localMatrix);
		archive->SetMatrix4("tc.worldMatrix", worldMatrix);
	}
}

void TransformComponent::Deserialize(KeyedArchive *archive)
{
	if(NULL != archive)
	{
		if(archive->IsKeyExists("tc.localMatrix")) localMatrix = archive->GetMatrix4("tc.localMatrix");
		if(archive->IsKeyExists("tc.worldMatrix")) localMatrix = archive->GetMatrix4("tc.worldMatrix");
	}

	Component::Deserialize(archive);
}

};