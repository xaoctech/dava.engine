#ifndef __DAVAENGINE_RENDER_SPATIAL_TREE_H__
#define	__DAVAENGINE_RENDER_SPATIAL_TREE_H__

#include "Base/BaseObject.h"
#include "Math/AABBox3.h"
//#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{

const static uint32 INVALID_TREE_NODE_INDEX = (uint32)(-1);
class RenderObject;
class Frustum;
class AbstractSpatialTree : public BaseObject
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
	struct QuadTreeNode //still basic implementation - later move it to more compact
	{
		enum eNodeType {NODE_LB=0, NODE_RB=1, NODE_LT=2, NODE_RT=3, NODE_NONE = 4};
		uint32 parent;
		uint32 children[4]; //think about allocating and freeing at groups of for
		float32 zMin, zMax;		
		int32 numChildNodes; 
		uint32 startClipPlane;
		bool dirtyZ;
		Vector<RenderObject *>  objects;
		QuadTreeNode();
	};

	

	Vector<QuadTreeNode> nodes;
	Vector<uint32> emptyNodes;
	
	AABBox3 worldBox;
	int32 maxTreeDepth;
	Frustum *currFrustum;

	List<int32> dirtyZNodes;
	
	/*to compare*/
	int32 objFrustrumCalls;	
	int32 nodeFrustrumCalls;
	int32 processClippingCalls;
	
	bool CheckObjectFitNode(const AABBox3& objBox, const AABBox3& nodeBox);
	bool CheckBoxIntersectBranch(const AABBox3& objBox, float32 xmin, float32 ymin, float32 xmax, float32 ymax);		
	bool CheckBoxIntersectChild(const AABBox3& objBox, const AABBox3& nodeBoxe, QuadTreeNode::eNodeType nodeType); //assuming it already fit parent!
	void UpdateChildBox(AABBox3 &parentBox, QuadTreeNode::eNodeType childType);
	void UpdateParentBox(AABBox3 &childBox, QuadTreeNode::eNodeType childType);	
	
	void DebugDrawNode(uint32 nodeId, AABBox3 box);
	void ProcessNodeClipping(uint32 nodeId, AABBox3& box, uint32 clippingFlags);	

	void AddObjectToNode(uint32 baseNodeId, int32 baseDepth, const AABBox3& box, RenderObject *object);
	static const int32 RECALCULATE_Z_PER_FRAME = 10;
	void RecalculateNodeZLimits(uint32 nodeId);

public:
	QuadTree(const AABBox3 &worldBox, int32 maxTreeDepth);
	
	virtual void AddObject(RenderObject *object);
	virtual void RemoveObject(RenderObject *object);	
	virtual void ObjectUpdated(RenderObject *object);  

	virtual void ProcessClipping(Frustum *frustum);
	virtual void UpdateTree();
	virtual void DebugDraw();
};

} //namespace DAVA
#endif