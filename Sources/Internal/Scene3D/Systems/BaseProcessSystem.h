#ifndef __DAVAENGINE_SCENE3D_BASEPROCESSSYSTEM_H__
#define __DAVAENGINE_SCENE3D_BASEPROCESSSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{

class Component;
class BaseProcessSystem : public SceneSystem
{
public:
	BaseProcessSystem(uint32 componentId);	

	virtual void AddEntity(SceneNode * entity);
	virtual void RemoveEntity(SceneNode * entity);

protected:
	Vector<Component*> components;
	uint32 processingComponentId;
};

}

#endif //__DAVAENGINE_SCENE3D_BASEPROCESSSYSTEM_H__
