#ifndef __ENTITY_MODIFICATION_SYSTEM_H__
#define __ENTITY_MODIFICATION_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

class SceneCollisionSystem;

class EntityModificationSystem : public DAVA::SceneSystem
{
public:
	EntityModificationSystem(DAVA::Scene * scene, SceneCollisionSystem *collisionSystem);
	~EntityModificationSystem();

	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

protected:
	SceneCollisionSystem *sceneCollisionSystem;

	bool modifState;
	DAVA::Vector2 modifStartPoint;
};

#endif //__ENTITY_MODIFICATION_SYSTEM_H__
