#ifndef __BOX_PROPERTY_CONTROL_H__
#define __BOX_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "MeshInstancePropertyControl.h"

using namespace DAVA;

class BoxPropertyControl : public MeshInstancePropertyControl
{
public:
	BoxPropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~BoxPropertyControl();

	virtual void ReadFrom(SceneNode * sceneNode);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor); 
};

#endif //__BOX_PROPERTY_CONTROL_H__
