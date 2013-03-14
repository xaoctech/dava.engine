#ifndef __DAVAENGINE_TRANSFORM_COMPONENT_H__
#define __DAVAENGINE_TRANSFORM_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Entity/Component.h"

namespace DAVA 
{

class Entity;

class TransformComponent : public Component
{
public:
    TransformComponent();
    virtual ~TransformComponent();

    IMPLEMENT_COMPONENT_TYPE(TRANSFORM_COMPONENT);

	inline Matrix4 * GetWorldTransformPtr();
    inline const Matrix4 & GetWorldTransform();
	inline const Matrix4 & GetLocalTransform();
	Matrix4 & ModifyLocalTransform();

	inline int32 GetIndex();

	void SetLocalTransform(const Matrix4 * transform);
	void SetParent(Entity * node);

    virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);

private:
	Matrix4 localMatrix;
	Matrix4 worldMatrix;
	Matrix4 * parentMatrix;
	Entity * parent; //Entity::parent should be removed

	int32 index;

	friend class TransformSystem;
    
public:

    INTROSPECTION_EXTEND(TransformComponent, Component,
        MEMBER(localMatrix, "Local Transform", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR_READONLY | INTROSPECTION_EDITOR)
        MEMBER(worldMatrix, "World Transform", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR_READONLY | INTROSPECTION_EDITOR)
        MEMBER(parentMatrix, "Parent Matrix", INTROSPECTION_SERIALIZABLE)
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
