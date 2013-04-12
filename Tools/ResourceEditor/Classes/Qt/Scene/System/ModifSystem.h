#ifndef __ENTITY_MODIFICATION_SYSTEM_H__
#define __ENTITY_MODIFICATION_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

class SceneCollisionSystem;
class SceneCameraSystem;
class EntityGroup;
class HoodSystem;

enum EntityModifMode
{
	EM_MODE_OFF,
	EM_MODE_MOVE,
	EM_MODE_ROTATE,
	EM_MODE_SCALE
};

enum EntityModifAxis
{
	EM_AXIS_NONE = 0,

	EM_AXIS_X = 0x1,
	EM_AXIS_Y = 0x2,
	EM_AXIS_Z = 0x4,
	EM_AXIS_XY = EM_AXIS_X | EM_AXIS_Y,
	EM_AXIS_XZ = EM_AXIS_X | EM_AXIS_Z,
	EM_AXIS_YZ = EM_AXIS_Y | EM_AXIS_Z
};

class EntityModificationSystem : public DAVA::SceneSystem
{
	friend class SceneEditorProxy;

public:
	EntityModificationSystem(DAVA::Scene * scene, SceneCollisionSystem *colSys, SceneCameraSystem *camSys, HoodSystem *hoodSys);
	~EntityModificationSystem();

protected:
	SceneCollisionSystem *collisionSystem;
	SceneCameraSystem *cameraSystem;
	HoodSystem* hoodSystem;

	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

	int GetModifAxis() const;
	void SetModifAxis(int axis);

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
	bool modified;

	int  curMode;
	int  curAxis;

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
	int modifPivotPoint;

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
