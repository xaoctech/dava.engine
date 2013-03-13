#ifndef __DAVAENGINE_SCENE3D_UPDATABLESYSTEM_H__
#define __DAVAENGINE_SCENE3D_UPDATABLESYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{

class IUpdatableBeforeTransform;
class IUpdatableAfterTransform;
class UpdateSystem : public SceneSystem
{
public:
	UpdateSystem(Scene * scene);
	virtual void Process();
	virtual void AddEntity(Entity * entity);
	virtual void RemoveEntity(Entity * entity);

	void UpdatePreTransform();
	void UpdatePostTransform();

private:
	Vector<IUpdatableBeforeTransform*> updatesBeforeTransform;
	Vector<IUpdatableAfterTransform*> updatesAfterTransform;
};

}

#endif //__DAVAENGINE_SCENE3D_UPDATABLESYSTEM_H__