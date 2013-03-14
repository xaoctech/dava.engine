#ifndef __CAMERA_PROPERTY_CONTROL_H__
#define __CAMERA_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "NodesPropertyControl.h"

class CameraPropertyControl : public NodesPropertyControl
{
public:
	CameraPropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~CameraPropertyControl();

	virtual void ReadFrom(Entity * sceneNode);
    
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
};

#endif //__CAMERA_PROPERTY_CONTROL_H__
