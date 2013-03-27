#ifndef __DAVAENGINE_SCENE3D_DEBUGRENDERSYSTEM_H__
#define __DAVAENGINE_SCENE3D_DEBUGRENDERSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{

class Camera;

class DebugRenderSystem : public SceneSystem
{
public:
	DebugRenderSystem(Scene * scene);
    ~DebugRenderSystem();

	virtual void Process();
	virtual void AddEntity(Entity * entity);
	virtual void RemoveEntity(Entity * entity);
    
    void SetCamera(Camera * camera);

private:
	Vector<Entity*> entities;
    Camera * camera;
};

}

#endif //__DAVAENGINE_SCENE3D_DEBUGRENDERSYSTEM_H__