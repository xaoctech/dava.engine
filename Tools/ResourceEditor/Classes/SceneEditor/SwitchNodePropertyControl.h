#ifndef __SWITCHNODE_PROPERTY_CONTROL_H__
#define __SWITCHNODE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "NodesPropertyControl.h"

class SwitchNodePropertyControl : public NodesPropertyControl
{
public:
	SwitchNodePropertyControl(const Rect & rect, bool createNodeProperties);

	virtual void ReadFrom(Entity * sceneNode);

	virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey);

};

#endif //__SWITCHNODE_PROPERTY_CONTROL_H__
