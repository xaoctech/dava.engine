#include "Scene3D/Components/Transform.h"
#include "Scene3D/SceneNode.h"

namespace DAVA
{

void Transform::SetLocalTransform(const Matrix4 * transform)
{
	*localTransform = *transform;
	if(!parent)
	{
		*worldTransform = *transform;
	}
}

void Transform::SetParent(SceneNode * node)
{
	parent = node;
	TransformSystem * system = TransformSystem::Instance();

	if(node)
	{
		system->LinkTransform(parent->GetTransform()->GetIndex(), GetIndex());
	}
	else
	{
		system->UnlinkTransform(GetIndex());
	}
}



};