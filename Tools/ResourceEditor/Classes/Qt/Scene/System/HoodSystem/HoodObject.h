#ifndef __HOOD_OBJECT_H__
#define __HOOD_OBJECT_H__

#include "Scene/System/HoodSystem/HoodCollObject.h"
#include "Scene/SceneTypes.h"

struct HoodObject 
{
	HoodObject(DAVA::float32 baseSize);
	virtual ~HoodObject();

	DAVA::float32 baseSize;
	DAVA::Color colorX; // axis X
	DAVA::Color colorY; // axis X
	DAVA::Color colorZ; // axis X
	DAVA::Color colorS; // axis selected

	virtual void UpdatePos(const DAVA::Vector3 &pos);
	virtual void UpdateScale(const DAVA::float32 &scale);
	virtual void Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis) = 0;

	HoodCollObject* CreateLine(const DAVA::Vector3 &from, const DAVA::Vector3 &to);

	DAVA::Vector<HoodCollObject*> collObjects;
};

#endif
