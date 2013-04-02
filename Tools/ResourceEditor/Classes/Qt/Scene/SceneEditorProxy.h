#ifndef __SCENE_EDITOR_PROXY_H__
#define __SCENE_TAB_WIDGET_H__

#include <QObject>
#include "UI/UIEvent.h"
#include "Scene3D/Scene.h"

class EntityModifSystem;
class SceneCameraSystem;
class SceneGridSystem;
class EntityCollisionSystem;

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

protected:
	DAVA::String curScenePath;

protected:
	EntityModifSystem *modifSystem;
	EntityCollisionSystem *collisionSystem;
	SceneCameraSystem* sceneCameraSystem;
	SceneGridSystem* sceneGridSystem;

	virtual void Draw();
	bool SceneLoad();
	bool SceneSave();
};

#endif