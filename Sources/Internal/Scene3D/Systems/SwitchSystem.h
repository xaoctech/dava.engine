#ifndef __DAVAENGINE_SCENE3D_SWITCHSYSTEM_H__
#define __DAVAENGINE_SCENE3D_SWITCHSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{

class SwitchSystem : public SceneSystem
{
public:
	SwitchSystem(Scene * scene);

	virtual void Process();
	virtual void ImmediateEvent(Entity * entity, uint32 event);

private:
	Set<Entity*> updatableEntities;

	void SetVisibleHierarchy(Entity * entity, bool visible);
};

}

#endif //__DAVAENGINE_SCENE3D_SWITCHSYSTEM_H__
