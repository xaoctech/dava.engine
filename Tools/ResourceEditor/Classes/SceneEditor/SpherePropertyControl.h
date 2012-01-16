#ifndef __SPHERE_PROPERTY_CONTROL_H__
#define __SPHERE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "MeshInstancePropertyControl.h"

class SpherePropertyControl : public MeshInstancePropertyControl
{
public:
	SpherePropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~SpherePropertyControl();

	virtual void ReadFrom(SceneNode * sceneNode);
	virtual void WriteTo(SceneNode * sceneNode);
    
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
};

#endif //__SPHERE_PROPERTY_CONTROL_H__
