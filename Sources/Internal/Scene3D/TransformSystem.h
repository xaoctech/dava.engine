#ifndef __DAVAENGINE_TRANSFORM_SYSTEM_H__
#define __DAVAENGINE_TRANSFORM_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Math/MathConstants.h"
#include "Math/Matrix4.h"
#include "Base/Singleton.h"
#include "Entity/SceneSystem.h"

namespace DAVA 
{

class SceneNode;
class Transform;

class TransformSystem : public SceneSystem
{
public:
	TransformSystem();
	~TransformSystem();

    Transform * CreateTransform();

	virtual void ImmediateUpdate(SceneNode * entity);

    void DeleteTransform(Transform * transform);
    void LinkTransform(int32 parentIndex, int32 childIndex);
	void UnlinkTransform(int32 childIndex);
    
    virtual void Process();

private:
    void SortAndThreadSplit();
    
	Vector<SceneNode*> updatableEntities;

	void HierahicNeedUpdate(SceneNode * entity);
	void HierahicAddToUpdate(SceneNode * entity);

	void HierahicFindUpdatableTranform(SceneNode * entity);

	int32 passedNodes;
	int32 multipliedNodes;
};

};

#endif //__DAVAENGINE_TRANSFORM_SYSTEM_H__
