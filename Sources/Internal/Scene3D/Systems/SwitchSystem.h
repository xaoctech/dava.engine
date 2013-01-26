#ifndef __DAVAENGINE_SCENE3D_SWITCHSYSTEM_H__
#define __DAVAENGINE_SCENE3D_SWITCHSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{

class SwitchSystem : public SceneSystem
{
public:
	SwitchSystem();

	virtual void Process();
	virtual void ImmediateEvent(SceneNode * entity, uint32 event);

private:
	Set<SceneNode*> updatableEntities;

	void SetUpdatableHierarchy(SceneNode * entity, bool updatable);
};

}

#endif //__DAVAENGINE_SCENE3D_SWITCHSYSTEM_H__
