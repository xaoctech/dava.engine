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

	virtual void ReadFrom(Entity * sceneNode);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor); 

protected:
    Vector<String> types;
};

#endif //__LIGHT_PROPERTY_CONTROL_H__
