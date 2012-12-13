#ifndef __DAVAENGINE_TRANSFORM_H__
#define __DAVAENGINE_TRANSFORM_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Components/TransformSystem.h"

namespace DAVA 
{

class SceneNode;

class Transform
{
public:
	inline Matrix4 * GetWorldTransform();
	inline Matrix4 * GetLocalTransform();
	inline int32 GetIndex();

	void SetLocalTransform(const Matrix4 * transform);
	void SetParent(SceneNode * node);

private:
	Matrix4 localMatrix;
	Matrix4 worldMatrix;
	Matrix4 * parentMatrix;
	SceneNode * parent; //SceneNode::parent should be removed

	int32 index;

	friend class TransformSystem;
};

Matrix4 * Transform::GetWorldTransform()
{
	return &worldMatrix;
}

Matrix4 * Transform::GetLocalTransform()
{
	return &localMatrix;
}

int32 Transform::GetIndex()
{
	return index;
}

};

#endif //__DAVAENGINE_TRANSFORM_H__
