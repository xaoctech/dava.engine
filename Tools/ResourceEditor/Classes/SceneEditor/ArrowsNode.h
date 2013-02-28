#ifndef __RESOURCEEDITORQT__ARROWSNODE__
#define __RESOURCEEDITORQT__ARROWSNODE__

#include "DAVAEngine.h"
#include "BulletObject.h"

using namespace DAVA;

class ArrowsNode;

class ArrowsRenderObject: public RenderObject
{
public:
	ArrowsRenderObject(ArrowsNode* node);
};

class ArrowsRenderBatch: public RenderBatch
{
public:
	ArrowsRenderBatch(ArrowsNode* node);

	const FastName & GetOwnerLayerName();
	virtual void Draw(Camera * camera);

protected:
	ArrowsNode* node;
};

class ArrowsNode: public SceneNode
{
public:
	enum eModAxis
    {
        AXIS_X = 0,
        AXIS_Y,
        AXIS_Z,
        AXIS_XY,
        AXIS_YZ,
        AXIS_XZ,
		AXIS_NONE,
		AXIS_COUNT = AXIS_NONE
	};

private:
	enum eAxisColors
	{
		COLOR_X = 0,
		COLOR_Y,
		COLOR_Z,
		COLOR_XY_X,
		COLOR_XY_Y,
		COLOR_YZ_Y,
		COLOR_YZ_Z,
		COLOR_XZ_X,
		COLOR_XZ_Z,

		COLORS_COUNT
	};

public:
	ArrowsNode();
	virtual ~ArrowsNode();

	virtual void Draw();
	virtual void ProcessMouse(UIEvent * event, const Vector3& cursorPos, const Vector3& cursorDir);

	Vector3 GetPosition();
	void SetPosition(const Vector3& newPosition);
	
	void SetVisible(bool visible);
	bool IsVisible();

	void SetActive(bool active);
	bool IsActive();

	eModAxis GetModAxis();

	void UpdateSize(const Vector3& camPos);

private:
	void PrepareColors(Color* colors);
	void DrawPrism(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4, const Vector3& p5);

private:
	Vector3 position;
	float32 scaleFactor;

	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* collisionDispatcher;
	btAxisSweep3* axisSweep;
	btCollisionWorld* collisionWorld;

	uint32 selected;
	bool visible;
	bool active;
};

#endif /* defined(__RESOURCEEDITORQT__ARROWSNODE__) */
