#ifndef __DAVAENGINE_TRANSFORM_SYSTEM_H__
#define __DAVAENGINE_TRANSFORM_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Math/MathConstants.h"
#include "Math/Matrix4.h"
#include "Base/Singleton.h"

namespace DAVA 
{

class SceneNode;
class Transform;

class TransformSystem : public Singleton<TransformSystem>
{
public:
	TransformSystem();
	~TransformSystem();

    Transform * CreateTransform();
    Transform * GetTransformWithIncrement(Transform * transform);
	Transform * CloneTransform(Transform * transform);

	void NeedUpdate(SceneNode * entity);

    void DeleteTransform(Transform * transform);
    void LinkTransform(int32 parentIndex, int32 childIndex);
	void UnlinkTransform(int32 childIndex);
    
    void Process();

private:
    void SortAndThreadSplit();
    
	Vector<SceneNode*> updatableEntities;

	void HierahicNeedUpdate(SceneNode * entity);
};

};

#endif //__DAVAENGINE_TRANSFORM_SYSTEM_H__
