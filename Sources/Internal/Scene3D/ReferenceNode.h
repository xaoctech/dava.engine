#ifndef __DAVAENGINE_REFERENCE_NODE_H__
#define __DAVAENGINE_REFERENCE_NODE_H__

#include "Scene3D/SceneNode.h"

namespace DAVA
{

class ReferenceNode : public SceneNode
{
public:
	ReferenceNode();

	virtual void Update(float32 timeElapsed);
	virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2);
	virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFileV2);

	virtual void GetDataNodes(Set<DataNode*> & dataNodes);

	SceneNode * nodeToAdd;
};

};

#endif //__DAVAENGINE_REFERENCE_NODE_H__
