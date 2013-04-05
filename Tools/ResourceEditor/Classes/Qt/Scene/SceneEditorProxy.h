#ifndef __SCENE_EDITOR_PROXY_H__
#define __SCENE_TAB_WIDGET_H__

#include <QObject>
#include "UI/UIEvent.h"
#include "Scene3D/Scene.h"

class EntityModificationSystem;
class SceneCameraSystem;
class SceneGridSystem;
class SceneCollisionSystem;
class SceneSelectionSystem;

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

	EntityModificationSystem *modifSystem;
	SceneCameraSystem *sceneCameraSystem;
	SceneCollisionSystem *sceneCollisionSystem;
	SceneGridSystem *sceneGridSystem;
	SceneSelectionSystem *sceneSelectionSystem;

protected:
	DAVA::String curScenePath;

	virtual void Draw();
	bool SceneLoad();
	bool SceneSave();
};

#endif