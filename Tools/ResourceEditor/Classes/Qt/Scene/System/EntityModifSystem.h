#ifndef __ENTITY_MODIFICATION_SYSTEM_H__
#define __ENTITY_MODIFICATION_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

#include "Scene/System/EntityModif/Hood.h"

class SceneCollisionSystem;
class SceneCameraSystem;
class EntityGroup;

class EntityModificationSystem : public DAVA::SceneSystem
{
	friend class SceneEditorProxy;

public:
	enum Mode
	{
		MODIF_OFF,
		MODIF_MOVE,
		MODIF_ROTATE,
		MODIF_SCALE
	};

	enum Axis
	{
		AXIS_X,
		AXIS_Y,
		AXIS_Z,
		AXIS_XY,
		AXIS_XZ,
		AXIS_YZ
	};

	enum PivotPoint
	{
		ENTITY_CENTER,
		SELECTION_CENTER
	};

	EntityModificationSystem(DAVA::Scene * scene, SceneCollisionSystem *colSys, SceneCameraSystem *camSys);
	~EntityModificationSystem();

protected:
	SceneCollisionSystem *collisionSystem;
	SceneCameraSystem *cameraSystem;

	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

protected:
	struct EntityToModify
	{
		DAVA::Entity* entity;
		DAVA::Matrix4 originalTransform;
		DAVA::Vector3 originalCenter;
		DAVA::Matrix4 moveToZeroPos;
		DAVA::Matrix4 moveFromZeroPos;
	};

	bool inModifState;
	int  curMode;
	int  curAxis;
	int  curPivotPoint;
	Hood curHood;

	// starting modification pos
	DAVA::Vector3 modifStartPos3d;
	DAVA::Vector2 modifStartPos2d;

	// entities to modify
	DAVA::Vector<EntityToModify> modifEntities;

	// values calculated, when starting modification
	DAVA::Vector3 modifEntitiesCenter;
	DAVA::AABBox3 modifEntitiesBbox;
	DAVA::Matrix4 moveToZeroPosRelativeCenter;
	DAVA::Matrix4 moveFromZeroPosRelativeCenter;
	DAVA::Vector2 rotateNormal;
	DAVA::Vector3 rotateAround;

	void BeginModification(const EntityGroup *entities);
	void EndModification();

	DAVA::Entity* GetFirstIntersectedEntity(const EntityGroup *gr1, const EntityGroup *gr2);
	DAVA::Vector3 CamCursorPosToModifPos(const DAVA::Vector3 &camPosition, const DAVA::Vector3 &camPointDirection, const DAVA::Vector3 &planePoint);
	DAVA::Vector2 Cam2dProjection(const DAVA::Vector3 &from, const DAVA::Vector3 &to);

	void Move(const DAVA::Vector3 &newPos3d);
	void Rotate(const DAVA::Vector2 &newPos2d);
	void Scale(const DAVA::Vector2 &newPos2d);
};

#endif //__ENTITY_MODIFICATION_SYSTEM_H__
