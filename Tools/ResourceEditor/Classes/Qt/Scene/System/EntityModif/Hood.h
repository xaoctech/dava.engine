#ifndef __ENTITY_MODIFICATION_SYSTEM_HOOD_H__
#define __ENTITY_MODIFICATION_SYSTEM_HOOD_H__

// bullet
#include "bullet/btBulletCollisionCommon.h"

// framework
#include "Base/BaseTypes.h"
#include "UI/UIEvent.h"

class Hood
{
public:
	enum HoodType
	{
		HOOD_NORMAL,
		HOOD_MOVE,
		HOOD_ROTATE,
		HOOD_SCALE
	};

	Hood(SceneCameraSystem *camSys);
	~Hood();

	void SetType(int type);
	int GetType() const;

	void SetPosition(DAVA::Vector3 pos);
	void SetScale(DAVA::float32 scale);

	void SetSelectedAxis(int axis);
	int GetSelectedAxis() const;

	int MouseOverAxis() const;

	void Draw() const;
	void Update() const;
	void RayTest(const DAVA::Vector3 &from, const DAVA::Vector3 &to);

protected:
	int curType;
	DAVA::Vector3 curPos;
	DAVA::float32 curScale;
	int curAxis;

	SceneCameraSystem *cameraSystem;

	void CreateCollObjects();
	void RemoveCollObjects();


	void AddLineShape(const DAVA::Vector3 &from, const DAVA::Vector3 &to);

private:
	btCollisionWorld* collWorld;
	btAxisSweep3* collBroadphase;
	btDefaultCollisionConfiguration* collConfiguration;
	btCollisionDispatcher* collDispatcher;
	btIDebugDraw* collDebugDraw;

	struct HoodObject
	{
		btCollisionObject* btObject;
		btCollisionShape* btShape;
		btTransform btObjTransform;
		DAVA::Color originalColor;
		DAVA::Color selectedColor;
		int representAxis;
	};

	DAVA::Vector<HoodObject> moveHood;
	DAVA::Vector<HoodObject> rotateHood;
	DAVA::Vector<HoodObject> scaleHood;
};

#endif // __ENTITY_MODIFICATION_SYSTEM_HOOD_H__
