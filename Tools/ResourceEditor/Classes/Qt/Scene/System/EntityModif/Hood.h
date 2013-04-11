#ifndef __ENTITY_MODIFICATION_SYSTEM_HOOD_H__
#define __ENTITY_MODIFICATION_SYSTEM_HOOD_H__

// bullet
#include "bullet/btBulletCollisionCommon.h"

// framework
#include "Base/BaseTypes.h"
#include "UI/UIEvent.h"

struct HoodObject;

class Hood
{
public:
	Hood();
	~Hood();

	void SetType(int type);
	int GetType() const;

	void SetPosition(const DAVA::Vector3 &pos);
	void MovePosition(const DAVA::Vector3 &offset);
	void SetScale(DAVA::float32 scale);

	void SetSelectedAxis(int axis);
	int GetSelectedAxis() const;

	int MouseOverAxis() const;

	void Draw() const;
	void RayTest(const DAVA::Vector3 &from, const DAVA::Vector3 &to);

protected:
	int curType;
	DAVA::Vector3 curPos;
	DAVA::float32 curScale;
	int curAxis;

	void CreateCollObjects();
	void UpdateCollObjects();
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

	HoodObject* CreateLine(const DAVA::Vector3 &from, const DAVA::Vector3 &to, DAVA::float32 weight = 0.05f);
	void Destroy(HoodObject *hoodObject);
};

struct HoodObject
{
	HoodObject() : btObject(NULL), btShape(NULL) {};

	btCollisionObject* btObject;
	btCollisionShape* btShape;
	DAVA::Vector3 initialOffset;
	DAVA::Vector3 curOffset;
	int representAxis;

	virtual void UpdatePos(const DAVA::Vector3 &pos);
	virtual void Draw() = 0;
};

struct HoodLine : public HoodObject
{
	DAVA::Vector3 from;
	DAVA::Vector3 to;

	virtual void Draw();
};


#endif // __ENTITY_MODIFICATION_SYSTEM_HOOD_H__
