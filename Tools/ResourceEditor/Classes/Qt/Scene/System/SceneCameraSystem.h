#ifndef __SCENE_CAMERA_SYSTEM_H__
#define __SCENE_CAMERA_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "Render/Highlevel/Camera.h"
#include "UI/UIEvent.h"

class SceneCameraSystem : public DAVA::SceneSystem
{
public:
	SceneCameraSystem(DAVA::Scene * scene);
	~SceneCameraSystem();

	void SetMoveSeep(DAVA::float32 speed);
	DAVA::float32 GetMoveSpeed();

	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

protected:
	DAVA::float32 curSpeed;
	DAVA::Camera* curSceneCamera;

	void ProcessMove(float timeElapsed);
	void ProcessRotate(DAVA::UIEvent *event);
};

#endif