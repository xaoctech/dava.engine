#ifndef __SCENE_EDITOR_PROXY_H__
#define __SCENE_EDITOR_PROXY_H__

#include <QObject>
#include "UI/UIEvent.h"
#include "Scene3D/Scene.h"
#include "Base/StaticSingleton.h"

class EntityModificationSystem;
class SceneCameraSystem;
class SceneGridSystem;
class SceneCollisionSystem;
class SceneSelectionSystem;
class HoodSystem;

class SceneEditorSignals;

class SceneEditorProxy : public DAVA::Scene
{
public:
	SceneEditorProxy();
	~SceneEditorProxy();

	bool Open(const DAVA::String &path);
	bool Save();
	bool Save(const DAVA::String &path);

	DAVA::String GetScenePath();
	void SetScenePath(const DAVA::String &newScenePath);

	virtual void Update(float timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);

	// this function should be called each time UI3Dview changes its position
	// viewport rect is used to cals ray from camera to any 2d point on this viewport
	void SetViewportRect(const DAVA::Rect &newViewportRect);

public:
	SceneEditorSignals *sceneSignals;

	SceneCameraSystem *cameraSystem;
	SceneCollisionSystem *collisionSystem;
	SceneGridSystem *gridSystem;
	HoodSystem *hoodSystem;
	SceneSelectionSystem *selectionSystem;
	EntityModificationSystem *modifSystem;

protected:
	DAVA::String curScenePath;

	virtual void Draw();
	bool SceneLoad();
	bool SceneSave();
};

class SceneEditorSignals : public QObject, public DAVA::StaticSingleton<SceneEditorSignals>
{
	Q_OBJECT

public:
	void EmitMouseOver(SceneEditorProxy *scene, DAVA::Entity *entity);
	void EmitMouseOverSelection(SceneEditorProxy *scene, DAVA::Entity *entity);

	void EmitSelected(SceneEditorProxy *scene, DAVA::Entity *entity);
	void EmitDeselected(SceneEditorProxy *scene, DAVA::Entity *entity);

signals:
	void MouseOver(SceneEditorProxy *scene, DAVA::Entity *entity);
	void MouseOverSelection(SceneEditorProxy *scene, DAVA::Entity *entity);

	void Selected(SceneEditorProxy *scene, DAVA::Entity *entity);
	void Deselected(SceneEditorProxy *scene, DAVA::Entity *entity);
};

#endif // __SCENE_EDITOR_PROXY_H__
