#ifndef __SCENE_SELECTION_SYSTEM_H__
#define __SCENE_SELECTION_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

class SceneCollisionSystem;

class SceneSelectionSystem : public DAVA::SceneSystem
{
public:
	enum DebugDrawFlags
	{
		DEBUG_DRAW_NOTHING = 0x0,

		SELECTION_DRAW_SHAPE = 0x1,
		SELECTION_FILL_SHAPE = 0x2,
		SELECTION_NO_DEEP_TEST = 0x4,

		DEBUG_DRAW_ALL = 0xFFFFFFFF
	};

	SceneSelectionSystem(DAVA::Scene * scene, SceneCollisionSystem *collisionSystem);
	~SceneSelectionSystem();

	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

	void SetSelectionDrawFlags(int flags);
	int GetSelectionDrawFlags();

	void SetSelection(DAVA::Entity *entity);
	void AddSelection(DAVA::Entity *entity);
	void RemSelection(DAVA::Entity *);

	DAVA::Entity* GetSelectionSingle();
	const DAVA::Vector<DAVA::Entity *>* GetSelectionsAll();

protected:
	int selectionDrawFlags;

	SceneCollisionSystem *sceneCollisionSystem;
	DAVA::Vector<DAVA::Entity *> curSelections;
};

#endif //__SCENE_SELECTION_SYSTEM_H__
