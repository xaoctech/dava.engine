#include "Scene/System/CollisionSystem.h"
#include "Scene/System/CollisionSystem/CollisionRenderObject.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/SceneEditorProxy.h"

// framework
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Entity.h"

SceneCollisionSystem::SceneCollisionSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
	, debugDrawFlags(DEBUG_DRAW_RAYTEST)
{
	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);

	objectsCollConf = new btDefaultCollisionConfiguration();
	objectsCollDisp = new btCollisionDispatcher(objectsCollConf);
	objectsBroadphase = new btAxisSweep3(worldMin,worldMax);
	objectsDebugDrawer = new SceneCollisionDebugDrawer();
	objectsDebugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	objectsCollWorld = new btCollisionWorld(objectsCollDisp, objectsBroadphase, objectsCollConf);
	objectsCollWorld->setDebugDrawer(objectsDebugDrawer);

	landCollConf = new btDefaultCollisionConfiguration();
	landCollDisp = new btCollisionDispatcher(landCollConf);
	landBroadphase = new btAxisSweep3(worldMin,worldMax);
	landDebugDrawer = new SceneCollisionDebugDrawer();
	landDebugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	landCollWorld = new btCollisionWorld(landCollDisp, landBroadphase, landCollConf);
	landCollWorld->setDebugDrawer(landDebugDrawer);
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

	QMapIterator<DAVA::Entity*, CollisionBaseObject*> i(entityToCollision);
	while(i.hasNext())
	{
		i.next();
		delete i.value();
	}
}

void SceneCollisionSystem::SetDebugDrawFlags(int flags)
{
	debugDrawFlags = flags;
}

int SceneCollisionSystem::GetDebugDrawFlags()
{
	return debugDrawFlags;
}

const EntityGroup* SceneCollisionSystem::RayTest(DAVA::Vector3 from, DAVA::Vector3 to)
{
	DAVA::Entity *retEntity = NULL;

	// check if cache is available 
	if(rayIntersectCached && lastRayFrom == from && lastRayTo == to)
	{
		return &rayIntersectedEntities;
	}

	// no cache. start ray new ray test
	lastRayFrom = from;
	lastRayTo = to;
	rayIntersectedEntities.Clear();

	btVector3 btFrom(from.x, from.y, from.z);
	btVector3 btTo(to.x, to.y, to.z);

	btCollisionWorld::AllHitsRayResultCallback btCallback(btFrom, btTo);
	objectsCollWorld->rayTest(btFrom, btTo, btCallback);

	if(btCallback.hasHit()) 
	{
		int foundCount = btCallback.m_collisionObjects.size();
		if(foundCount > 0)
		{
			for(int i = 0; i < foundCount; ++i)
			{
				btCollisionObject *btObj = btCallback.m_collisionObjects[i];
				DAVA::Entity *entity = collisionToEntity.value(btObj, NULL);

				if(NULL != entity)
				{
					rayIntersectedEntities.Add(entity, GetBoundingBox(entity));
				}
			}
		}
	}

	return &rayIntersectedEntities;
}

const EntityGroup* SceneCollisionSystem::RayTestFromCamera()
{
	SceneCameraSystem *cameraSystem	= ((SceneEditorProxy *) GetScene())->cameraSystem;

	DAVA::Vector3 camPos = cameraSystem->GetCameraPosition();
	DAVA::Vector3 camDir = cameraSystem->GetPointDirection(lastMousePos);

	DAVA::Vector3 traceFrom = camPos;
	DAVA::Vector3 traceTo = traceFrom + camDir * 1000.0f;

	return RayTest(traceFrom, traceTo);
}

DAVA::AABBox3 SceneCollisionSystem::GetBoundingBox(DAVA::Entity *entity)
{
	DAVA::AABBox3 aabox;
	if(NULL != entity)
	{

		CollisionBaseObject* collObj = entityToCollision.value(entity, NULL);
		if(NULL != collObj)
		{
			aabox = collObj->boundingBox;
		}
	}

	return aabox;
}

void SceneCollisionSystem::Update(DAVA::float32 timeElapsed)
{
	// reset cache on new frame
	rayIntersectCached = false;
}

void SceneCollisionSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	lastMousePos = event->point;
}

void SceneCollisionSystem::Draw()
{
	if(debugDrawFlags & DEGUG_DRAW_OBJECTS)
	{
		objectsCollWorld->debugDrawWorld();
	}

	if(debugDrawFlags & DEBUG_DRAW_LAND)
	{
		landCollWorld->debugDrawWorld();
	}

	if(debugDrawFlags & DEBUG_DRAW_RAYTEST)
	{
		int oldState = DAVA::RenderManager::Instance()->GetState();
		DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE | DAVA::RenderState::STATE_DEPTH_TEST);
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 0, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(lastRayFrom, lastRayTo);
		DAVA::RenderManager::Instance()->SetState(oldState);
	}
}

void SceneCollisionSystem::AddEntity(DAVA::Entity * entity)
{
	if(NULL != entity)
	{
		// build collision object for entity
		BuildFromEntity(entity);

		// build collision object for entitys childs
		for(int i = 0; i < entity->GetChildrenCount(); ++i)
		{
			AddEntity(entity->GetChild(i));
		}
	}
}

void SceneCollisionSystem::RemoveEntity(DAVA::Entity * entity)
{

}

void SceneCollisionSystem::BuildFromEntity(DAVA::Entity * entity)
{
	// check if this entity is landscape
	DAVA::Landscape *landscape = DAVA::GetLandscape(entity);
	if(NULL != landscape)
	{
		// landscape has its own collision word
		// TODO:
		// ...
	}
	else
	{
		// check if entity has render object. if so - build bullet object
		// from this render object
		DAVA::RenderObject *renderObject = DAVA::GetRenerObject(entity);
		if(NULL != renderObject)
		{
			CollisionBaseObject *collObj = new CollisionRenderObject(entity, objectsCollWorld, renderObject);
			entityToCollision[entity] = collObj;
			collisionToEntity[collObj->btObject] = entity;
		}
	}
}

SceneCollisionDebugDrawer::SceneCollisionDebugDrawer()
	: manager(DAVA::RenderManager::Instance())
	, helper(DAVA::RenderHelper::Instance())
	, dbgMode(0)
{ }

void SceneCollisionDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	DAVA::Vector3 davaFrom(from.x(), from.y(), from.z());
	DAVA::Vector3 davaTo(to.x(), to.y(), to.z());
	DAVA::Color davaColor(color.w(), color.z(), color.y(), color.x());

	manager->SetColor(davaColor);
	helper->DrawLine(davaFrom, davaTo);
}

void SceneCollisionDebugDrawer::drawContactPoint( const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color )
{ }

void SceneCollisionDebugDrawer::reportErrorWarning( const char* warningString )
{ }

void SceneCollisionDebugDrawer::draw3dText( const btVector3& location,const char* textString )
{ }

void SceneCollisionDebugDrawer::setDebugMode( int debugMode )
{
	dbgMode = debugMode;
}

int SceneCollisionDebugDrawer::getDebugMode() const
{
	return dbgMode;
}
