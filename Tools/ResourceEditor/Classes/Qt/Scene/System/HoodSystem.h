#ifndef __ENTITY_MODIFICATION_SYSTEM_HOOD_H__
#define __ENTITY_MODIFICATION_SYSTEM_HOOD_H__

// bullet
#include "bullet/btBulletCollisionCommon.h"

// framework
#include "Entity/SceneSystem.h"
#include "UI/UIEvent.h"

struct HoodObject;
class SceneCameraSystem;

class HoodSystem : public DAVA::SceneSystem
{
	friend class SceneEditorProxy;

public:
	HoodSystem(DAVA::Scene * scene, SceneCameraSystem *camSys);
	~HoodSystem();

	void SetType(int type);
	int GetType() const;

	DAVA::Vector3 GetPosition() const;
	void SetPosition(const DAVA::Vector3 &pos);
	void MovePosition(const DAVA::Vector3 &offset);

	void SetSelectedAxis(int axis);
	int GetSelectedAxis() const;
	int GetPassingAxis() const;

	void Lock();
	void Unlock();

	void Show();
	void Hide();

protected:
	bool locked;
	bool invisible;

	int curType;
	DAVA::Vector3 curPos;
	DAVA::Vector3 curOffset;
	DAVA::float32 curScale;
	int curAxis;
	int moseOverAxis;

	SceneCameraSystem *cameraSystem;

	virtual void Update(float timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();

	void SetScale(DAVA::float32 scale);

	void CreateCollObjects();
	void UpdateCollObjectsPos();
	void UpdateCollObjectsScale();
	void RemoveCollObjects();

private:
	btCollisionWorld* collWorld;
	btAxisSweep3* collBroadphase;
	btDefaultCollisionConfiguration* collConfiguration;
	btCollisionDispatcher* collDispatcher;
	btIDebugDraw* collDebugDraw;

	DAVA::Vector<HoodObject*> normalHood;
	DAVA::Vector<HoodObject*> moveHood;
	DAVA::Vector<HoodObject*> rotateHood;
	DAVA::Vector<HoodObject*> scaleHood;
	const DAVA::Vector<HoodObject*>* GetCurrentHood() const;

	HoodObject* CreateLine(const DAVA::Vector3 &from, const DAVA::Vector3 &to, DAVA::float32 weight = 0.3f);
	void Destroy(HoodObject *hoodObject);
};

struct HoodObject
{
	HoodObject() : btObject(NULL), btShape(NULL), representAxis(0), colorAxis(0) {};

	btCollisionObject* btObject;
	btCollisionShape* btShape;
	DAVA::Vector3 initialPos;
	DAVA::Vector3 initialOffset;
	DAVA::Vector3 scaledOffset;
	DAVA::Matrix4 rotate;
	DAVA::Vector3 curOffset;
	DAVA::float32 scale;
	int representAxis;
	int colorAxis;

	virtual void UpdatePos(const DAVA::Vector3 &pos);
	virtual void UpdateScale(const DAVA::float32 &scale);
	virtual void Draw() = 0;
};

struct HoodLine : public HoodObject
{
	DAVA::Vector3 from;
	DAVA::Vector3 to;

	virtual void Draw();
};


#endif // __ENTITY_MODIFICATION_SYSTEM_HOOD_H__
