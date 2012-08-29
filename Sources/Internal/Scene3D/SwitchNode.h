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
	virtual void AddNode(SceneNode * node);
	virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2);
	virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFileV2);

	void SetSwitchIndex(int32 switchIndex);
	int32 GetSwitchIndex();

private:
	int32 oldSwitchIndex;
	int32 newSwitchIndex;

	void ReapplySwitch();
};

};

#endif //__DAVAENGINE_SWITCH_NODE_H__
