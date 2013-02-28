#ifndef __RESOURCEEDITORQT__ARROWSNODE__
#define __RESOURCEEDITORQT__ARROWSNODE__

#include "DAVAEngine.h"
#include "BulletObject.h"

using namespace DAVA;

class ArrowsNode;

class ArrowsRenderBatch: public RenderBatch
{
protected:
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
	ArrowsRenderBatch(ArrowsNode* node);

	const FastName & GetOwnerLayerName();
	virtual void Draw(Camera * camera);

protected:
	ArrowsNode* node;

	void PrepareColors(Color* colors);
	void DrawPrism(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4, const Vector3& p5);
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

public:
	ArrowsNode();
	virtual ~ArrowsNode();

	virtual void ProcessMouse(UIEvent * event, const Vector3& cursorPos, const Vector3& cursorDir);

	void SetActive(bool active);
	bool IsActive();

	eModAxis GetModAxis();

private:
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* collisionDispatcher;
	btAxisSweep3* axisSweep;
	btCollisionWorld* collisionWorld;

	uint32 selected;
	bool active;
};

#endif /* defined(__RESOURCEEDITORQT__ARROWSNODE__) */
