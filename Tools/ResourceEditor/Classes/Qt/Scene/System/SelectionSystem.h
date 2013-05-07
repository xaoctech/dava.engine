#ifndef __SCENE_SELECTION_SYSTEM_H__
#define __SCENE_SELECTION_SYSTEM_H__

#include "Scene/EntityGroup.h"
#include "Scene/SceneTypes.h"

// framework
#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

class SceneCollisionSystem;
class HoodSystem;

class SceneSelectionSystem : public DAVA::SceneSystem
{
	friend class SceneEditorProxy;
	friend class EntityModificationSystem;

public:
	SceneSelectionSystem(DAVA::Scene * scene, SceneCollisionSystem *collSys, HoodSystem *hoodSys);
	~SceneSelectionSystem();

	void SetSelection(DAVA::Entity *entity);
	void AddSelection(DAVA::Entity *entity);
	void RemSelection(DAVA::Entity *entity);

	const EntityGroup* GetSelection() const;

	void SetDrawMode(int mode);
	int GetDrawMode() const;

	void SetPivotPoint(ST_PivotPoint pp);
	ST_PivotPoint GetPivotPoint() const;

protected:
	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

	EntityGroup GetSelecetableFromCollision(const EntityGroup *collisionEntities);
	EntityGroupItem GetSelectableEntity(DAVA::Entity* entity);

	void UpdateHoodPos() const;
	void SelectedItemsWereModified();

	DAVA::AABBox3 CalcAABox(DAVA::Entity *entity) const;

private:
	int drawMode;
	bool applyOnPhaseEnd;

	SceneCollisionSystem *collisionSystem;
	HoodSystem* hoodSystem;

	EntityGroup curSelections;
	DAVA::Entity *lastSelection;

	ST_PivotPoint curPivotPoint;
};

#endif //__SCENE_SELECTION_SYSTEM_H__
