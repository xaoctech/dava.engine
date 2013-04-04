#ifndef __SCENE_COLLISION_SYSTEM_H__
#define __SCENE_COLLISION_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "UI/UIEvent.h"
#include "bullet/btBulletCollisionCommon.h"

class SceneCollisionSystem : public DAVA::SceneSystem
{
public:
	SceneCollisionSystem(DAVA::Scene * scene);
	~SceneCollisionSystem();

	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

	virtual void AddEntity(DAVA::Entity * entity);
	virtual void RemoveEntity(DAVA::Entity * entity);

protected:
	/*
	btDefaultCollisionConfiguration* objectsCollConf;
	btCollisionDispatcher* objectsCollDisp;
	btAxisSweep3* objectsBroadphase;

	btDefaultCollisionConfiguration* landCollConf;
	btCollisionDispatcher* landCollDisp;
	btAxisSweep3* landBroadphase;
	*/
};

#endif // __SCENE_COLLISION_SYSTEM_H__