#ifndef __BOX_PROPERTY_CONTROL_H__
#define __BOX_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "MeshInstancePropertyControl.h"

class BoxPropertyControl : public MeshInstancePropertyControl
{
public:
	BoxPropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~BoxPropertyControl();

	virtual void ReadFrom(SceneNode * sceneNode);
	virtual void WriteTo(SceneNode * sceneNode);
    
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
};

#endif //__BOX_PROPERTY_CONTROL_H__
