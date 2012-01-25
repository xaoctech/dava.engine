#ifndef __SPHERE_PROPERTY_CONTROL_H__
#define __SPHERE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "MeshInstancePropertyControl.h"

using namespace DAVA;

class SpherePropertyControl : public MeshInstancePropertyControl
{
public:
	SpherePropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~SpherePropertyControl();

	virtual void ReadFrom(SceneNode * sceneNode);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor); 
};

#endif //__SPHERE_PROPERTY_CONTROL_H__
