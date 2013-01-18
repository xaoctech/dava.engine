#ifndef __DAVAENGINE_TRANSFORM_COMPONENT_H__
#define __DAVAENGINE_TRANSFORM_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Entity/Component.h"

namespace DAVA 
{

class SceneNode;

class TransformComponent : public Component
{
public:
    TransformComponent();
    ~TransformComponent();

    IMPLEMENT_COMPONENT_TYPE(TRANSFORM_COMPONENT);

	inline Matrix4 * GetWorldTransformPtr();
    inline const Matrix4 & GetWorldTransform();
	inline const Matrix4 & GetLocalTransform();
	Matrix4 & ModifyLocalTransform();

	inline int32 GetIndex();

	void SetLocalTransform(const Matrix4 * transform);
	void SetParent(SceneNode * node);
    virtual Component * Clone();

private:
	Matrix4 localMatrix;
	Matrix4 worldMatrix;
	Matrix4 * parentMatrix;
	SceneNode * parent; //SceneNode::parent should be removed

	int32 index;

	friend class TransformSystem;
    
public:

    INTROSPECTION_EXTEND(TransformComponent, Component,
        MEMBER(localMatrix, "Local Transform", INTROSPECTION_FLAG_SERIALIZABLE | INTROSPECTION_FLAG_EDITOR_READONLY)
        MEMBER(worldMatrix, "World Transform", INTROSPECTION_FLAG_SERIALIZABLE | INTROSPECTION_FLAG_EDITOR_READONLY)
    );
};

const Matrix4 & TransformComponent::GetWorldTransform()
{
	return worldMatrix;
}

const Matrix4 & TransformComponent::GetLocalTransform()
{
	return localMatrix;
}

int32 TransformComponent::GetIndex()
{
	return index;
}


Matrix4 * TransformComponent::GetWorldTransformPtr()
{
	return &worldMatrix;
}

};

#endif //__DAVAENGINE_TRANSFORM_COMPONENT_H__
