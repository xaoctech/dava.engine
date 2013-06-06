#ifndef __SCENE_COLLISION_SYSTEM_H__
#define __SCENE_COLLISION_SYSTEM_H__

#include <QMap>
#include "Scene/EntityGroup.h"
#include "Scene/SceneTypes.h"

// bullet
#include "bullet/btBulletCollisionCommon.h"

// framework
#include "UI/UIEvent.h"
#include "Entity/SceneSystem.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

class CollisionBaseObject;
class SceneCollisionDebugDrawer;

class SceneCollisionSystem : public DAVA::SceneSystem
{
	friend class SceneEditorProxy;
	friend class EntityModificationSystem;

public:
	SceneCollisionSystem(DAVA::Scene * scene);
	~SceneCollisionSystem();

	void SetDrawMode(int mode);
	int GetDebugDrawFlags();

	DAVA::AABBox3 GetBoundingBox(DAVA::Entity *entity);

	const EntityGroup* ObjectsRayTest(const DAVA::Vector3 &from, const DAVA::Vector3 &to);
	const EntityGroup* ObjectsRayTestFromCamera();

	bool LandRayTest(const DAVA::Vector3 &from,
					 const DAVA::Vector3 &to,
					 DAVA::Vector3& intersectionPoint);
	bool LandRayTestFromCamera(DAVA::Vector3& intersectionPoint);

	void UpdateCollisionObject(DAVA::Entity *entity);

protected:
	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

	virtual void AddEntity(DAVA::Entity * entity);
	virtual void RemoveEntity(DAVA::Entity * entity);

protected:
	int drawMode;

	DAVA::Vector3 lastRayFrom;
	DAVA::Vector3 lastRayTo;
	DAVA::Vector2 lastMousePos;

	EntityGroup rayIntersectedEntities;
	bool rayIntersectCached;

	DAVA::Vector3 lastLandRayFrom;
	DAVA::Vector3 lastLandRayTo;
	DAVA::Vector3 lastLandCollision;
	bool lastResult;

	btDefaultCollisionConfiguration* objectsCollConf;
	btCollisionDispatcher* objectsCollDisp;
	btAxisSweep3* objectsBroadphase;
	btCollisionWorld *objectsCollWorld;
	SceneCollisionDebugDrawer *objectsDebugDrawer;

	btDefaultCollisionConfiguration* landCollConf;
	btCollisionDispatcher* landCollDisp;
	btAxisSweep3* landBroadphase;
	btCollisionWorld *landCollWorld;
	SceneCollisionDebugDrawer *landDebugDrawer;

	QMap<DAVA::Entity*, CollisionBaseObject*> entityToCollision;
	QMap<btCollisionObject*, DAVA::Entity*> collisionToEntity;

	void BuildFromEntity(DAVA::Entity * entity);
	void DestroyFromEntity(DAVA::Entity * entity);
};

class SceneCollisionDebugDrawer : public btIDebugDraw
{
public:
	SceneCollisionDebugDrawer();

	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
	virtual void reportErrorWarning(const char* warningString);
	virtual void draw3dText(const btVector3& location,const char* textString);
	virtual void setDebugMode(int debugMode);
	virtual int	getDebugMode() const;

protected:
	int dbgMode;
	DAVA::RenderManager *manager;
	DAVA::RenderHelper *helper;
};

#endif // __SCENE_COLLISION_SYSTEM_H__