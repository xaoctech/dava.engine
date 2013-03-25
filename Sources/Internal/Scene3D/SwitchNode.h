#ifndef __DAVAENGINE_SWITCH_NODE_H__
#define __DAVAENGINE_SWITCH_NODE_H__

#include "Scene3D/Entity.h"

namespace DAVA
{

class SwitchNode : public Entity
{
public:
	SwitchNode();

	virtual Entity* Clone(Entity *dstNode = NULL);
	virtual void Update(float32 timeElapsed);
	virtual void AddNode(Entity * node);
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
