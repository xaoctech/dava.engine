#ifndef __ENTITY_MODIFICATION_SYSTEM_H__
#define __ENTITY_MODIFICATION_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

#include "Scene/SceneTypes.h"

class SceneCollisionSystem;
class SceneCameraSystem;
class EntityGroup;
class HoodSystem;

class EntityModificationSystem : public DAVA::SceneSystem
{
	friend class SceneEditorProxy;

public:
	EntityModificationSystem(DAVA::Scene * scene, SceneCollisionSystem *colSys, SceneCameraSystem *camSys, HoodSystem *hoodSys);
	~EntityModificationSystem();

	ST_Axis GetModifAxis() const;
	void SetModifAxis(ST_Axis axis);

	ST_ModifMode GetModifMode() const;
	void SetModifMode(ST_ModifMode mode);

protected:
	SceneCollisionSystem *collisionSystem;
	SceneCameraSystem *cameraSystem;
	HoodSystem* hoodSystem;

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
	bool modified;

	ST_ModifMode curMode;
	ST_Axis curAxis;

	// starting modification pos
	DAVA::Vector3 modifStartPos3d;
	DAVA::Vector2 modifStartPos2d;

	// entities to modify
	DAVA::Vector<EntityToModify> modifEntities;

	// values calculated, when starting modification
	ST_PivotPoint modifPivotPoint;
	DAVA::Vector3 modifEntitiesCenter;
	DAVA::Matrix4 moveToZeroPosRelativeCenter;
	DAVA::Matrix4 moveFromZeroPosRelativeCenter;
	DAVA::Vector2 rotateNormal;
	DAVA::Vector3 rotateAround;
	DAVA::float32 crossXY;
	DAVA::float32 crossXZ;
	DAVA::float32 crossYZ;

	void BeginModification(const EntityGroup *entities);
	void EndModification();

	DAVA::Vector3 CamCursorPosToModifPos(const DAVA::Vector3 &camPosition, const DAVA::Vector3 &camPointDirection, const DAVA::Vector3 &planePoint);
	DAVA::Vector2 Cam2dProjection(const DAVA::Vector3 &from, const DAVA::Vector3 &to);

	DAVA::Vector3 Move(const DAVA::Vector3 &newPos3d);
	DAVA::float32 Rotate(const DAVA::Vector2 &newPos2d);
	DAVA::float32 Scale(const DAVA::Vector2 &newPos2d);

	void MoveDone(const DAVA::Vector2 &newPos3d);
	void RotateDone(const DAVA::Vector2 &newPos2d);
	void ScaleDone(const DAVA::Vector2 &newPos2d);
};

#endif //__ENTITY_MODIFICATION_SYSTEM_H__
