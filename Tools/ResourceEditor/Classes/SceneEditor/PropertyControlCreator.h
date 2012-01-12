#ifndef __PROPERTY_CONTROL_CREATOR_H__
#define __PROPERTY_CONTROL_CREATOR_H__

#include "DAVAEngine.h"
using namespace DAVA;

class NodesPropertyControl;
class PropertyControlCreator
{
public:
	static NodesPropertyControl * CreateControlForNode(SceneNode * sceneNode, const Rect & rect, bool createNodeProperties);
};

#endif //__PROPERTY_CONTROL_CREATOR_H__
