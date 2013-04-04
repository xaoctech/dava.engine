#include "Scene/System/SceneCollisionSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"

SceneCollisionSystem::SceneCollisionSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
{
	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);

	objectsCollConf = new btDefaultCollisionConfiguration();
	objectsCollDisp = new btCollisionDispatcher(objectsCollConf);
	objectsBroadphase = new btAxisSweep3(worldMin,worldMax);
	objectsCollWorld = new btCollisionWorld(objectsCollDisp, objectsBroadphase, objectsCollConf);

	landCollConf = new btDefaultCollisionConfiguration();
	landCollDisp = new btCollisionDispatcher(landCollConf);
	landBroadphase = new btAxisSweep3(worldMin,worldMax);
	landCollWorld = new btCollisionWorld(landCollDisp, landBroadphase, landCollConf);
}

SceneCollisionSystem::~SceneCollisionSystem()
{
	DAVA::SafeDelete(objectsCollWorld);
	DAVA::SafeDelete(objectsBroadphase);
	DAVA::SafeDelete(objectsCollDisp);
	DAVA::SafeDelete(objectsCollConf);

	DAVA::SafeDelete(landCollWorld); 
	DAVA::SafeDelete(landBroadphase);
	DAVA::SafeDelete(landCollDisp);
	DAVA::SafeDelete(landCollConf);
}

void SceneCollisionSystem::Update(DAVA::float32 timeElapsed)
{

}

void SceneCollisionSystem::ProcessUIEvent(DAVA::UIEvent *event)
{

}

void SceneCollisionSystem::Draw()
{

}

void SceneCollisionSystem::AddEntity(DAVA::Entity * entity)
{
	if(NULL != entity)
	{
		DAVA::Landscape *landscape = DAVA::GetLandscape(entity);

	}
}

void SceneCollisionSystem::RemoveEntity(DAVA::Entity * entity)
{

}
