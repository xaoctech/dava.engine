#ifndef __SCENE_EDITOR_PROXY_H__
#define __SCENE_EDITOR_PROXY_H__

#include <QObject>
#include "UI/UIEvent.h"
#include "Scene3D/Scene.h"
#include "Base/StaticSingleton.h"

#include "Scene/System/CameraSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/GridSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/System/ModifSystem.h"

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

	bool Load(const DAVA::FilePath &path);
	bool Save(const DAVA::FilePath &path);
	bool Save();

	DAVA::FilePath GetScenePath();
	void SetScenePath(const DAVA::FilePath &newScenePath);

	void PostUIEvent(DAVA::UIEvent *event);

	// this function should be called each time UI3Dview changes its position
	// viewport rect is used to calc. ray from camera to any 2d point on this viewport
	void SetViewportRect(const DAVA::Rect &newViewportRect);

protected:
	DAVA::FilePath curScenePath;

	virtual void Update(float timeElapsed);
	virtual void Draw();
};

#endif // __SCENE_EDITOR_PROXY_H__
