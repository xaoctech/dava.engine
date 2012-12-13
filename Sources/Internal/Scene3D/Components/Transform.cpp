#include "Scene3D/Components/Transform.h"
#include "Scene3D/SceneNode.h"

namespace DAVA
{

void Transform::SetLocalTransform(const Matrix4 * transform)
{
	localMatrix = *transform;
	if(!parent)
	{
		worldMatrix = *transform;
	}
}

void Transform::SetParent(SceneNode * node)
{
	parent = node;

	if(node)
	{
		parentMatrix = node->GetTransform()->GetWorldTransform();
	}
	else
	{
		parentMatrix = 0;
	}
}

};