#include "Scene/SceneEditorProxy.h"
#include "Scene/System/SceneCameraSystem.h"
#include "Scene/System/SceneGridSystem.h"
#include "Scene/System/SceneCollisionSystem.h"
#include "Scene/System/SceneSelectionSystem.h"
#include "Scene/System/EntityModifSystem.h"

// framework
#include "Scene3D/SceneFileV2.h"

SceneEditorProxy::SceneEditorProxy()
	: Scene()
{
	sceneCameraSystem = new SceneCameraSystem(this);
	AddSystem(sceneCameraSystem, 0);

	sceneGridSystem = new SceneGridSystem(this);
	AddSystem(sceneGridSystem, 0);

	sceneCollisionSystem = new SceneCollisionSystem(this);
	AddSystem(sceneCollisionSystem, 0);

	sceneSelectionSystem = new SceneSelectionSystem(this, sceneCollisionSystem);
	AddSystem(sceneSelectionSystem, 0);

	modifSystem = new EntityModificationSystem(this, sceneCollisionSystem);
	AddSystem(modifSystem, 0);
}

SceneEditorProxy::~SceneEditorProxy()
{
	SafeDelete(sceneSelectionSystem);
	SafeDelete(sceneCollisionSystem);
	SafeDelete(sceneGridSystem);
	SafeDelete(sceneCameraSystem);
}

bool SceneEditorProxy::Open(const DAVA::String &path)
{
	bool ret = false;

	if(curScenePath != path)
	{
		curScenePath = path;
		ret = SceneLoad();
	}

	return ret;
}

bool SceneEditorProxy::Save()
{
	return Save(curScenePath);
}

bool SceneEditorProxy::Save(const DAVA::String &path)
{
	bool ret = false;

	if(!curScenePath.empty())
	{
		ret = SceneSave();
	}

	return ret;
}

DAVA::String SceneEditorProxy::GetScenePath()
{
	return curScenePath;
}

void SceneEditorProxy::SetScenePath(const DAVA::String &newScenePath)
{
	curScenePath = newScenePath;
}

void SceneEditorProxy::Update(float timeElapsed)
{
	Scene::Update(timeElapsed);

	sceneGridSystem->Update(timeElapsed);
	sceneCameraSystem->Update(timeElapsed);
	sceneCollisionSystem->Update(timeElapsed);
	sceneSelectionSystem->Update(timeElapsed);
	modifSystem->Update(timeElapsed);
}

void SceneEditorProxy::ProcessUIEvent(DAVA::UIEvent *event)
{
	sceneGridSystem->ProcessUIEvent(event);
	sceneCameraSystem->ProcessUIEvent(event);
	sceneCollisionSystem->ProcessUIEvent(event);
	sceneSelectionSystem->ProcessUIEvent(event);
	modifSystem->ProcessUIEvent(event);
}

void SceneEditorProxy::SetViewportRect(const DAVA::Rect &newViewportRect)
{
	sceneCameraSystem->SetViewportRect(newViewportRect);
}

void SceneEditorProxy::Draw()
{
	Scene::Draw();

	sceneGridSystem->Draw();
	sceneCameraSystem->Draw();
	sceneCollisionSystem->Draw();
	sceneSelectionSystem->Draw();
	modifSystem->Draw();
}

bool SceneEditorProxy::SceneLoad()
{
	bool ret = false;

	Entity * rootNode = GetRootNode(curScenePath);
	if(rootNode)
	{
		ret = true;

		DAVA::Vector<DAVA::Entity*> tmpEntities;
		int entitiesCount = rootNode->GetChildrenCount();

		tmpEntities.reserve(entitiesCount);

		// remember all child pointers, but don't add them to scene in this cycle
		// because when entity is adding it is automatically removing from its old hierarchy
		for (DAVA::int32 i = 0; i < entitiesCount; ++i)
		{
			tmpEntities.push_back(rootNode->GetChild(i));
		}

		// now we can safely add entities into our hierarchy
		for (DAVA::int32 i = 0; i < (DAVA::int32) tmpEntities.size(); ++i)
		{
			AddNode(tmpEntities[i]);
		}
	}

	return ret;
}

bool SceneEditorProxy::SceneSave()
{
	// TODO:
	// ...
	// 
	// 

	/*
	DAVA::SceneFileV2 *file = new DAVA::SceneFileV2();
	file->EnableDebugLog(false);

	DAVA::SceneFileV2::eError err = file->SaveScene(curScenePath, this);
	ret = (DAVA::SceneFileV2::ERROR_NO_ERROR == err);

	SafeRelease(file);
	*/

	return false;
}
