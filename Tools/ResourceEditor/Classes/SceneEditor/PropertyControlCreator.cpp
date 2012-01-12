#include "PropertyControlCreator.h"
#include "NodesPropertyControl.h"
#include "LightPropertyControl.h"

NodesPropertyControl * PropertyControlCreator::CreateControlForNode(SceneNode * sceneNode, const Rect & rect, bool createNodeProperties)
{
	LightNode * light = dynamic_cast<LightNode *>(sceneNode);
	if(light)
	{
		return new LightPropertyControl(rect, createNodeProperties);
	}

	return new NodesPropertyControl(rect, createNodeProperties);
}
