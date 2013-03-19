#ifndef __DAVAENGINE_TRANSFORM_SYSTEM_H__
#define __DAVAENGINE_TRANSFORM_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Math/MathConstants.h"
#include "Math/Matrix4.h"
#include "Base/Singleton.h"
#include "Entity/SceneSystem.h"

namespace DAVA 
{

class Entity;
class Transform;

class TransformSystem : public SceneSystem
{
public:
	TransformSystem(Scene * scene);
	~TransformSystem();

    Transform * CreateTransform();

	virtual void ImmediateEvent(Entity * entity, uint32 event);
	virtual void RemoveEntity(Entity * entity);

    void DeleteTransform(Transform * transform);
    void LinkTransform(int32 parentIndex, int32 childIndex);
	void UnlinkTransform(int32 childIndex);
    
    virtual void Process();

private:
    void SortAndThreadSplit();
    
	Vector<Entity*> updatableEntities;

	void EntityNeedUpdate(Entity * entity);
	void HierahicAddToUpdate(Entity * entity);

	void HierahicFindUpdatableTransform(Entity * entity, bool forcedUpdate = false);

	int32 passedNodes;
	int32 multipliedNodes;
};

};

#endif //__DAVAENGINE_TRANSFORM_SYSTEM_H__
