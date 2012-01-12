#ifndef __LIGHT_PROPERTY_CONTROL_H__
#define __LIGHT_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "NodesPropertyControl.h"

class LightPropertyControl : public NodesPropertyControl
{
public:
	LightPropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~LightPropertyControl();

	virtual void ReadFrom(SceneNode * sceneNode);
	virtual void WriteTo(SceneNode * sceneNode);
};

#endif //__LIGHT_PROPERTY_CONTROL_H__
