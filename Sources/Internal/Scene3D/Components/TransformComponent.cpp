#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/SceneNode.h"

namespace DAVA
{
    
TransformComponent::TransformComponent()
{
    localMatrix = Matrix4::IDENTITY;
	worldMatrix = Matrix4::IDENTITY;
	parentMatrix = 0;
	parent = 0;
}
    
TransformComponent::~TransformComponent()
{
    
}

Component * TransformComponent::Clone()
{
    TransformComponent * newTransform = new TransformComponent();
    newTransform->localMatrix = this->localMatrix;
    newTransform->worldMatrix = this->worldMatrix;
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
}

void TransformComponent::SetParent(SceneNode * node)
{
	parent = node;

	if(node)
	{
		parentMatrix = node->GetTransformComponent()->GetWorldTransform();
	}
	else
	{
		parentMatrix = 0;
	}
}

};