#ifndef __DAVAENGINE_RENDER_SPATIAL_TREE_H__
#define	__DAVAENGINE_RENDER_SPATIAL_TREE_H__

#include "Base/BaseTypes.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{

const static uint32 INVALID_TREE_NODE_INDEX = (uint32)(-1);

class AbstractSpatialTree
{
public:	
	virtual void AddObject(RenderObject *object) = 0;
	virtual void RemoveObject(RenderObject *object) = 0;	
	virtual void ObjectUpdated(RenderObject *object) = 0;  
	virtual void ProcessClipping(Frustum *frustum) = 0;
	virtual void UpdateTree() = 0; //theoretically for some trees that require re balance notify it's good time to do it
	virtual void DebugDraw() = 0;
};

class QuadTree : public AbstractSpatialTree
{
	struct QuadTreeNode
	{
		enum eNodeType {NODE_LB=0, NODE_RB=1, NODE_LT=2, NODE_RT=3, NODE_NONE = 4};
		uint32 parent;
		uint32 children[4];
		List<RenderObject *> objects;
	};

	

	Vector<QuadTreeNode> nodes;
	Vector<uint32> emptyNodes;
	
	AABBox3 worldBox;
	int32 maxTreeDepth;

	void CheckRemoveBranch(uint32 nodeIndex);
	bool CheckBoxIntersectBranch(const AABBox3& objBox, float32 xmin, float32 ymin, float32 xmax, float32 ymax);

public:
	QuadTree(const AABBox3& worldBox, int32 maxTreeDepth);
	virtual void AddObject(RenderObject *object);
	virtual void RemoveObject(RenderObject *object);	
	virtual void ObjectUpdated(RenderObject *object);  
	virtual void ProcessClipping(Frustum *frustum);
	virtual void UpdateTree();
	virtual void DebugDraw();
};

} //namespace DAVA
#endif