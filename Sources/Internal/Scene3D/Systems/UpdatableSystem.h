#ifndef __DAVAENGINE_SCENE3D_UPDATABLESYSTEM_H__
#define __DAVAENGINE_SCENE3D_UPDATABLESYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{

class IUpdatablePreTransform;
class IUpdatablePostTransform;
class UpdatableSystem : public SceneSystem
{
public:
	UpdatableSystem();
	virtual void Process();
	virtual void AddEntity(SceneNode * entity);
	virtual void RemoveEntity(SceneNode * entity);

	void UpdatePreTransform();
	void UpdatePostTransform();

private:
	Vector<IUpdatablePreTransform*> updatesPreTransform;
	Vector<IUpdatablePostTransform*> updatesPostTransform;
};

}

#endif //__DAVAENGINE_SCENE3D_UPDATABLESYSTEM_H__