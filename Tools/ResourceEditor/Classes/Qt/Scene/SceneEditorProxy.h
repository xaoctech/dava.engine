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

class SceneEditorProxy : public DAVA::Scene
{
public:
	SceneEditorProxy();
	~SceneEditorProxy();

	SceneCameraSystem *cameraSystem;
	SceneCollisionSystem *collisionSystem;
	SceneGridSystem *gridSystem;
	HoodSystem *hoodSystem;
	SceneSelectionSystem *selectionSystem;
	EntityModificationSystem *modifSystem;

	bool Load(const DAVA::String &path);
	bool Save(const DAVA::String &path);
	bool Save();

	DAVA::String GetScenePath();
	void SetScenePath(const DAVA::String &newScenePath);

	void PostUIEvent(DAVA::UIEvent *event);

	// this function should be called each time UI3Dview changes its position
	// viewport rect is used to calc. ray from camera to any 2d point on this viewport
	void SetViewportRect(const DAVA::Rect &newViewportRect);

protected:
	DAVA::String curScenePath;

	virtual void Update(float timeElapsed);
	virtual void Draw();

	bool SceneLoad();
	bool SceneSave();
};

#endif // __SCENE_EDITOR_PROXY_H__
