#ifndef __SCENE_SELECTION_SYSTEM_H__
#define __SCENE_SELECTION_SYSTEM_H__

#include "Scene/EntityGroup.h"

// framework
#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

class SceneCollisionSystem;

class SceneSelectionSystem : public DAVA::SceneSystem
{
	friend class SceneEditorProxy;

public:
	enum SelectionDrawFlags
	{
		DEBUG_DRAW_NOTHING = 0x0,

		SELECTION_DRAW_SHAPE = 0x1,
		SELECTION_FILL_SHAPE = 0x2,
		SELECTION_NO_DEEP_TEST = 0x4,

		DEBUG_DRAW_ALL = 0xFFFFFFFF
	};

	SceneSelectionSystem(DAVA::Scene * scene, SceneCollisionSystem *collisionSystem);
	~SceneSelectionSystem();

	void SetSelection(DAVA::Entity *entity);
	void AddSelection(DAVA::Entity *entity);
	void RemSelection(DAVA::Entity *entity);

	const EntityGroup* GetSelection() const;

	void SetSelectionDrawFlags(int flags);
	int GetSelectionDrawFlags() const;

protected:
	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

protected:
	int selectionDrawFlags;
	bool applyOnPhaseEnd;

	SceneCollisionSystem *collisionSystem;

	EntityGroup curSelections;
	DAVA::Entity *lastSelection;
};

#endif //__SCENE_SELECTION_SYSTEM_H__
