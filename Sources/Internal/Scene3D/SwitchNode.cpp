#include "Scene3D/SwitchNode.h"

namespace DAVA
{

SwitchNode::SwitchNode()
:	oldSwitchIndex(0),
	newSwitchIndex(0)
{

}

SceneNode* SwitchNode::Clone(SceneNode *dstNode /*= NULL*/)
{
	if (!dstNode) 
	{
		dstNode = new SwitchNode();
	}

	return SceneNode::Clone(dstNode);
}

void SwitchNode::SetSwitchIndex(int32 _switchIndex)
{
	newSwitchIndex = _switchIndex;
}

void SwitchNode::Update()
{
	if(oldSwitchIndex != newSwitchIndex)
	{
		int32 childrenCound = GetChildrenCount();
		for(int32 i = 0; i < childrenCound; ++i)
		{
			GetChild(i)->SetUpdatable(newSwitchIndex == i);
		}

		oldSwitchIndex = newSwitchIndex;
	}
}

}
