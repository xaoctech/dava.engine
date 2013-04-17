#ifndef __LODNODE_PROPERTY_CONTROL_H__
#define __LODNODE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "NodesPropertyControl.h"

class LodNodePropertyControl : public NodesPropertyControl
{
public:
	LodNodePropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~LodNodePropertyControl();

//    virtual void WillDisappear();
    
	virtual void ReadFrom(Entity * sceneNode);
    
//    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
//    virtual void OnDistancePropertyChanged(PropertyList *forList, const String &forKey, float32 newValue, int32 index);
//    virtual void OnSliderPropertyChanged(PropertyList *forList, const String &forKey, float32 newValue);
};

#endif //__LODNODE_PROPERTY_CONTROL_H__
