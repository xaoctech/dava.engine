#ifndef __DAVAENGINE_SWITCH_NODE_H__
#define __DAVAENGINE_SWITCH_NODE_H__

#include "Scene3D/SceneNode.h"

namespace DAVA
{

class SwitchNode : public SceneNode
{
public:
	SwitchNode();

	virtual SceneNode* Clone(SceneNode *dstNode = NULL);
	virtual void Update(float32 timeElapsed);

	void SetSwitchIndex(int32 switchIndex);

private:
	int32 oldSwitchIndex;
	int32 newSwitchIndex;
};

};

#endif //__DAVAENGINE_SWITCH_NODE_H__
