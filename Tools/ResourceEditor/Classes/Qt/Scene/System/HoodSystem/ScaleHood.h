#ifndef __SCALE_HOOD_H__
#define __SCALE_HOOD_H__

#include "Scene/System/HoodSystem/HoodObject.h"

struct ScaleHood : public HoodObject
{
	ScaleHood();
	~ScaleHood();

	virtual void Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis);

	HoodCollObject *axisX;
	HoodCollObject *axisY;
	HoodCollObject *axisZ;

	HoodCollObject *axisXY;
	HoodCollObject *axisXZ;
	HoodCollObject *axisYZ;
};

#endif // __SCALE_HOOD_H__
