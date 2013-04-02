#include "Scene/System/SceneCameraSystem.h"

// framework
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Scene.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

SceneCameraSystem::SceneCameraSystem(DAVA::Scene * scene)
	: SceneSystem(scene)
	, curSceneCamera(NULL)
	, curSpeed(35.0)
{
	// add debug cameras
	// there already can be other cameras in scene
	if(NULL != scene)
	{
		DAVA::Camera *mainCamera = NULL;
		DAVA::Camera *topCamera = NULL;

		mainCamera = new DAVA::Camera();
		mainCamera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
		mainCamera->SetPosition(DAVA::Vector3(0.0f, 0.0f, 0.0f));
		mainCamera->SetTarget(DAVA::Vector3(0.0f, 1.0f, 0.0f));
		mainCamera->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);

		DAVA::Entity *mainCameraEntity = new DAVA::Entity();
		mainCameraEntity->SetName("editor.main-camera");
		mainCameraEntity->AddComponent(new DAVA::CameraComponent(mainCamera));
		scene->AddNode(mainCameraEntity);
		scene->AddCamera(mainCamera);

		topCamera = new DAVA::Camera();
		topCamera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
		topCamera->SetPosition(DAVA::Vector3(0.0f, 0.0f, 200.0f));
		topCamera->SetTarget(DAVA::Vector3(0.0f, 250.0f, 0.0f));

		DAVA::Entity *topCameraEntity = new DAVA::Entity();
		topCameraEntity->SetName("editor.debug-camera");
		topCameraEntity->AddComponent(new DAVA::CameraComponent(topCamera));
		scene->AddNode(topCameraEntity);
		scene->AddCamera(topCamera);

		// set current default camera
		if(NULL == scene->GetCurrentCamera())
		{
			scene->SetCurrentCamera(topCamera);
		}

		SafeRelease(mainCamera);
		SafeRelease(topCamera);
	}
}

SceneCameraSystem::~SceneCameraSystem()
{
	SafeRelease(curSceneCamera);
}

void SceneCameraSystem::SetMoveSeep(DAVA::float32 speed)
{
	curSpeed = speed;
}

DAVA::float32 SceneCameraSystem::GetMoveSpeed()
{
	return curSpeed;
}

void SceneCameraSystem::Update(float timeElapsed)
{
	DAVA::Scene *scene = GetScene();
	if(NULL != scene)
	{
		DAVA::Camera* camera = scene->GetCurrentCamera();

		// is current camera in scene changed?
		if(curSceneCamera != camera)
		{
			// remember current scene camera
			SafeRelease(curSceneCamera);
			curSceneCamera = camera;
			SafeRetain(curSceneCamera);

			if(NULL != curSceneCamera)
			{
				// TODO:
				// some stuf for new camera
			}
		}

		ProcessMove(timeElapsed);
	}
}

void SceneCameraSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	ProcessRotate(event);

	// TODO:
	// Other event, like camera speed, or switch to next camera
	// ...
}

void SceneCameraSystem::Draw()
{
	// Nothing to draw
}

void SceneCameraSystem::ProcessMove(DAVA::float32 timeElapsed)
{
	if(NULL != curSceneCamera)
	{
		DAVA::float32 moveSpeed = curSpeed * timeElapsed;        

		if(1) // TODO: check for key modifiers
		{
			DAVA::KeyboardDevice *kd = DAVA::InputSystem::Instance()->GetKeyboard();

			if(kd->IsKeyPressed(DAVA::DVKEY_UP) || kd->IsKeyPressed(DAVA::DVKEY_W))
			{
				DAVA::Vector3 pos = curSceneCamera->GetPosition();
				DAVA::Vector3 dir = curSceneCamera->GetDirection();

				dir.Normalize();
				pos += dir * moveSpeed;
				curSceneCamera->SetPosition(pos);
				curSceneCamera->SetDirection(dir);    // right now required because camera rebuild direction to target, and if position & target is equal after set position it produce wrong results
			}

			if(kd->IsKeyPressed(DAVA::DVKEY_LEFT) || kd->IsKeyPressed(DAVA::DVKEY_A))
			{
				DAVA::Vector3 pos = curSceneCamera->GetPosition();
				DAVA::Vector3 dir = curSceneCamera->GetDirection();
				DAVA::Vector3 left = curSceneCamera->GetLeft();

				pos -= left * moveSpeed;
				curSceneCamera->SetPosition(pos);
				curSceneCamera->SetDirection(dir);
			}


			if(kd->IsKeyPressed(DAVA::DVKEY_DOWN) || kd->IsKeyPressed(DAVA::DVKEY_S))
			{
				DAVA::Vector3 pos = curSceneCamera->GetPosition();
				DAVA::Vector3 dir = curSceneCamera->GetDirection();

				pos -= dir * moveSpeed;
				curSceneCamera->SetPosition(pos);
				curSceneCamera->SetDirection(dir);    // right now required because camera rebuild direction to target, and if position & target is equal after set position it produce wrong results
			}


			if(kd->IsKeyPressed(DAVA::DVKEY_RIGHT) || kd->IsKeyPressed(DAVA::DVKEY_D))
			{
				DAVA::Vector3 pos = curSceneCamera->GetPosition();
				DAVA::Vector3 dir = curSceneCamera->GetDirection();
				DAVA::Vector3 left = curSceneCamera->GetLeft();

				pos += left * moveSpeed;
				curSceneCamera->SetPosition(pos);
				curSceneCamera->SetDirection(dir);
			}
		}
	}
}

void SceneCameraSystem::ProcessRotate(DAVA::UIEvent *event)
{

}
