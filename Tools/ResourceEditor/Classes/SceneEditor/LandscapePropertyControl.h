#ifndef __LANDSCAPE_PROPERTY_CONTROL_H__
#define __LANDSCAPE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "NodesPropertyControl.h"

class LandscapePropertyControl : public NodesPropertyControl
{
public:
	LandscapePropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~LandscapePropertyControl();

	virtual void ReadFrom(SceneNode * sceneNode);
	virtual void WriteTo(SceneNode * sceneNode);

    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey);

    
protected:
    Vector<String> renderingModes;

};

#endif //__LANDSCAPE_PROPERTY_CONTROL_H__
