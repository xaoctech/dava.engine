
#include "Render/Highlevel/SpatialTree.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/RenderHelper.h"

namespace DAVA
{
QuadTree::QuadTreeNode::QuadTreeNode()
{
	parent=INVALID_TREE_NODE_INDEX;
	for (int32 i=0; i<4; i++)
		children[i] = INVALID_TREE_NODE_INDEX;
	numChildNodes = 0;
	startClipPlane = 0;
	zMin = AABBOX_INFINITY;
	zMax = -AABBOX_INFINITY;
}

QuadTree::QuadTree(const AABBox3& _worldBox, int32 _maxTreeDepth): worldBox(_worldBox), maxTreeDepth(_maxTreeDepth)
{	
	QuadTreeNode root;
	
	root.zMin = worldBox.min.z;
	root.zMax = worldBox.max.z;
	nodes.push_back(root);	
}

bool QuadTree::CheckBoxIntersectBranch(const AABBox3& objBox, float32 xmin, float32 ymin, float32 xmax, float32 ymax)
{
	
	if ((xmin>objBox.max.x)||(ymin>objBox.max.y)||(xmax<objBox.min.x)||(ymax<objBox.min.y))
		return false;
	return true;
}

bool QuadTree::CheckBoxIntersectChild(const AABBox3& objBox, const AABBox3& nodeBox, QuadTreeNode::eNodeType nodeType)
{
	//note - this code assumes box already intersects parent
	switch (nodeType)
	{
	case QuadTreeNode::NODE_LB:
		return !((((nodeBox.min.x+nodeBox.max.x)*0.5f)<objBox.min.x)||(((nodeBox.min.y+nodeBox.max.y)*0.5)<objBox.min.y));
	case QuadTreeNode::NODE_RB:
		return !((((nodeBox.min.x+nodeBox.max.x)*0.5f)>objBox.max.x)||(((nodeBox.min.y+nodeBox.max.y)*0.5)<objBox.min.y));
	case QuadTreeNode::NODE_LT:
		return !((((nodeBox.min.x+nodeBox.max.x)*0.5f)<objBox.min.x)||(((nodeBox.min.y+nodeBox.max.y)*0.5)>objBox.max.y));
	case QuadTreeNode::NODE_RT:
		return !((((nodeBox.min.x+nodeBox.max.x)*0.5f)>objBox.max.x)||(((nodeBox.min.y+nodeBox.max.y)*0.5)>objBox.max.y));
	}			
	return false;
}

void QuadTree::UpdateChildBox(AABBox3 &parentBox, QuadTreeNode::eNodeType childType)
{
	switch (childType)
	{
	case  QuadTreeNode::NODE_LB:
		parentBox.max.x = (parentBox.max.x+parentBox.min.x)*0.5f;
		parentBox.max.y = (parentBox.max.y+parentBox.min.y)*0.5f;
		break;
	case  QuadTreeNode::NODE_RB:
		parentBox.min.x = (parentBox.max.x+parentBox.min.x)*0.5f;
		parentBox.max.y = (parentBox.max.y+parentBox.min.y)*0.5f;
		break;
	case  QuadTreeNode::NODE_LT:
		parentBox.max.x = (parentBox.max.x+parentBox.min.x)*0.5f;
		parentBox.min.y = (parentBox.max.y+parentBox.min.y)*0.5f;
		break;
	case  QuadTreeNode::NODE_RT:
		parentBox.min.x = (parentBox.max.x+parentBox.min.x)*0.5f;
		parentBox.min.y = (parentBox.max.y+parentBox.min.y)*0.5f;
		break;
	}
}

void QuadTree::UpdateParentBox(AABBox3 &childtBox, QuadTreeNode::eNodeType childType)
{
	switch (childType)
	{
	case  QuadTreeNode::NODE_LB:
		childtBox.max.x += (childtBox.max.x-childtBox.min.x);
		childtBox.max.y += (childtBox.max.y-childtBox.min.y);
		break;
	case  QuadTreeNode::NODE_RB:
		childtBox.min.x -= (childtBox.max.x-childtBox.min.x);
		childtBox.max.y += (childtBox.max.y-childtBox.min.y);
		break;
	case  QuadTreeNode::NODE_LT:
		childtBox.max.x += (childtBox.max.x-childtBox.min.x);
		childtBox.min.y -= (childtBox.max.y-childtBox.min.y);
		break;
	case  QuadTreeNode::NODE_RT:
		childtBox.min.x -= (childtBox.max.x-childtBox.min.x);
		childtBox.min.y -= (childtBox.max.y-childtBox.min.y);
		break;
	}
}

void QuadTree::AddObjectToNode(uint32 baseNodeId, int32 baseDepth, const AABBox3& box, RenderObject *object)
{
	uint32 currIndex = baseNodeId;	
	const AABBox3& objBox = object->GetWorldBoundingBox();
		
	
	float32 zMin = objBox.min.z;
	float32 zMax = objBox.max.z;		
	if (zMin<nodes[currIndex].zMin)
		nodes[currIndex].zMin = zMin;
	if (zMax>nodes[currIndex].zMax)
		nodes[currIndex].zMax = zMax;	

	bool placeHere = (baseDepth == maxTreeDepth);
	QuadTreeNode::eNodeType fitNode = QuadTreeNode::NODE_NONE;
	if (!placeHere)
	{
		for (int32 i=0; i<QuadTreeNode::NODE_NONE; i++)
		{
			if (CheckBoxIntersectChild(objBox, box, QuadTreeNode::eNodeType(i)))
			{
				if (fitNode==QuadTreeNode::NODE_NONE)
					fitNode=QuadTreeNode::eNodeType(i);
				else
				{
					placeHere = true;
					break;
				}
			}
		}
	}
	
	if (placeHere)
	{
		nodes[currIndex].objects.push_back(object);
		object->SetTreeNodeIndex(currIndex);	
	}
	else
	{
		if (nodes[currIndex].children[fitNode]==INVALID_TREE_NODE_INDEX) //set child node if not exist
		{
			int32 newNodeIndex;
			if (emptyNodes.size()) //take from empty
			{
				newNodeIndex = emptyNodes.back();
				emptyNodes.pop_back();
				nodes[newNodeIndex] = QuadTreeNode(); //reset it here
			}
			else //or create new node
			{
				newNodeIndex = nodes.size();
				nodes.resize(newNodeIndex+1);				
			}
			nodes[newNodeIndex].parent = currIndex;

			nodes[currIndex].children[fitNode] = newNodeIndex;
			nodes[currIndex].numChildNodes++;

		}
		AABBox3 childBox = box;
		UpdateChildBox(childBox, fitNode);
		AddObjectToNode(nodes[currIndex].children[fitNode], baseDepth+1, childBox, object);		
	}		
}

void QuadTree::RecalculateNodeZLimits(uint32 nodeId)
{
	QuadTreeNode& currNode = nodes[nodeId];
	currNode.zMin = AABBOX_INFINITY;
	currNode.zMax = -AABBOX_INFINITY;
	for (int32 i=0; i<QuadTreeNode::NODE_NONE; i++)
	{
		if (nodes[nodeId].children[i]!=INVALID_TREE_NODE_INDEX)
		{
			if (nodes[currNode.children[i]].zMin<currNode.zMin)
				currNode.zMin = nodes[currNode.children[i]].zMin;
			if (nodes[currNode.children[i]].zMax>currNode.zMax)
				currNode.zMax = nodes[currNode.children[i]].zMax;
		}
	}
	for (int32 i=0; i<currNode.objects.size(); i++)
	{
		const AABBox3 &objBox = currNode.objects[i]->GetWorldBoundingBox();
		if (objBox.min.z<currNode.zMin)
			currNode.zMin = objBox.min.z;
		if (objBox.max.z>currNode.zMax)
			currNode.zMax = objBox.max.z;
	}
}

void QuadTree::AddObject(RenderObject *object)
{
	DVASSERT(object->GetTreeNodeIndex()==INVALID_TREE_NODE_INDEX);		
	const AABBox3& objBox = object->GetWorldBoundingBox();
	
	//special treatment for root - as it can contain objects outside the world
	if (!worldBox.IsInside(objBox))
	{
		//object is somehow outside the world - just add to root
		nodes[0].objects.push_back(object);
		object->SetTreeNodeIndex(0);
		return;
	}
	AddObjectToNode(0, 0, worldBox, object);			
}

void QuadTree::RemoveObject(RenderObject *object)
{
	uint32 currIndex = object->GetTreeNodeIndex();
	DVASSERT(currIndex!=INVALID_TREE_NODE_INDEX);	
	object->SetTreeNodeIndex(INVALID_TREE_NODE_INDEX);
	Vector<RenderObject*>::iterator it = std::find(nodes[currIndex].objects.begin(), nodes[currIndex].objects.end(), object);
	DVASSERT(it!=nodes[currIndex].objects.end());
	nodes[currIndex].objects.erase(it);	

	//update tree branch info	
	while(currIndex!=INVALID_TREE_NODE_INDEX) 
	{
		QuadTreeNode& currNode = nodes[currIndex];
		
		if ((currIndex!=0)&& //do not remove root node anyway
			currNode.objects.empty()&&
			(currNode.children[0]==INVALID_TREE_NODE_INDEX)&&
			(currNode.children[1]==INVALID_TREE_NODE_INDEX)&&
			(currNode.children[2]==INVALID_TREE_NODE_INDEX)&&
			(currNode.children[3]==INVALID_TREE_NODE_INDEX))
		{ //empty node - just remove it from tree
			emptyNodes.push_back(currIndex);
			for (int32 i=0; i<4; i++)
				if (nodes[currNode.parent].children[i]==currIndex)
					nodes[currNode.parent].children[i]=INVALID_TREE_NODE_INDEX;
			nodes[currNode.parent].numChildNodes--;
		}		
		else
		{			
			RecalculateNodeZLimits(currIndex);
		}
		currIndex = currNode.parent;
		
	}
}


void QuadTree::ObjectUpdated(RenderObject *object)
{	
	//for now just simple remove and add
	RemoveObject(object);
	AddObject(object);
	return;
	//return back here once through with everything else
	/*uint32 currIndex = object->GetTreeNodeIndex();
	DVASSERT(currIndex!=INVALID_TREE_NODE_INDEX);	
	//for now just simple remove and add
	object->SetTreeNodeIndex(INVALID_TREE_NODE_INDEX);
	Vector<RenderObject*>::iterator it = std::find(nodes[currIndex].objects.begin(), nodes[currIndex].objects.end(), object);
	DVASSERT(it!=nodes[currIndex].objects.end());
	nodes[currIndex].objects.erase(it);		
	
	//climb up
	//update tree branch info	
	while(currIndex!=0) 
	{
		QuadTreeNode& currNode = nodes[currIndex];

		//if node

		if ((currIndex!=0)&& //do not remove root node anyway
			currNode.objects.empty()&&
			(currNode.children[0]==INVALID_TREE_NODE_INDEX)&&
			(currNode.children[1]==INVALID_TREE_NODE_INDEX)&&
			(currNode.children[2]==INVALID_TREE_NODE_INDEX)&&
			(currNode.children[3]==INVALID_TREE_NODE_INDEX))
		{ //empty node - just remove it from tree
			emptyNodes.push_back(currIndex);
			for (int32 i=0; i<4; i++)
				if (nodes[currNode.parent].children[i]==currIndex)
					nodes[currNode.parent].children[i]=INVALID_TREE_NODE_INDEX;
		}		
		else
		{			
			RecalculateNodeZLimits(currIndex);
		}
		currIndex = currNode.parent;

	}*/
	
	
}

void QuadTree::ProcessNodeClipping(uint32 nodeId, AABBox3& box, uint32 clippingFlags)
{
	processClippingCalls++;
	QuadTreeNode& currNode = nodes[nodeId];	
	int32 clipBoxCount = currNode.numChildNodes;
	int32 objectsSize = currNode.objects.size();
	//count clipping boxes for this node
	for (int32 i = 0; i<objectsSize; ++i)
	{
		if (currNode.objects[i]->GetFlags()&RenderObject::CLIPPING_VISIBILITY_CRITERIA)
			clipBoxCount++;
	}
	if (clippingFlags&&(clipBoxCount>1)&&nodeId)
	{
		nodeFrustrumCalls++;
		if (currFrustum->Classify(box, clippingFlags, currNode.startClipPlane)==Frustum::EFR_OUTSIDE)
			return; //node box is outside - return
	}
	//process objects in current node	
	if (!clippingFlags) //node is fully inside frustum - no need to clip anymore
	{
		for (int32 i = 0; i<objectsSize; ++i)
		{
			currNode.objects[i]->AddFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
		}
	}
	else
	{		
		for (int32 i = 0; i<objectsSize; ++i)
		{			
			if ((currNode.objects[i]->GetFlags()&RenderObject::CLIPPING_VISIBILITY_CRITERIA)==RenderObject::CLIPPING_VISIBILITY_CRITERIA)
			{
				objFrustrumCalls++;
				uint32 objClipPlanes = clippingFlags;
				uint32 startPlane = currNode.startClipPlane; //not to break it by objects culling		
				if ((currNode.objects[i]->GetFlags()&RenderObject::ALWAYS_CLIPPING_VISIBLE)||(currFrustum->Classify(currNode.objects[i]->GetWorldBoundingBox(), objClipPlanes, startPlane)!=Frustum::EFR_OUTSIDE))
					currNode.objects[i]->AddFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
			}				
		}
	}
	
	//process children
	AABBox3 childBox;
	for (int32 i=0; i<QuadTreeNode::NODE_NONE; ++i)
	{
		if (nodes[nodeId].children[QuadTreeNode::eNodeType(i)]!=INVALID_TREE_NODE_INDEX)
		{
			childBox = box;
			UpdateChildBox(childBox, QuadTreeNode::eNodeType(i));
			childBox.min.z = nodes[nodes[nodeId].children[QuadTreeNode::eNodeType(i)]].zMin;
			childBox.max.z = nodes[nodes[nodeId].children[QuadTreeNode::eNodeType(i)]].zMax;			
			ProcessNodeClipping(nodes[nodeId].children[QuadTreeNode::eNodeType(i)], childBox, clippingFlags);
		}
	}	
}

void QuadTree::ProcessClipping(Frustum *frustum)
{
	currFrustum = frustum;
	objFrustrumCalls = 0;
	nodeFrustrumCalls = 0;
	processClippingCalls = 0;
	Frustum::planeCalls = 0;
	ProcessNodeClipping(0, worldBox, 0x3f); //root node is considered as always pass  - as objects out of worldBox are added here
	int32 totalCalls = objFrustrumCalls+nodeFrustrumCalls;
	uint32 frustumPlaneCalls = Frustum::planeCalls;
	
}
void QuadTree::UpdateTree()
{		

}

void QuadTree::DebugDraw()
{
	RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
	RenderManager::Instance()->SetState(RenderState::STATE_COLORMASK_ALL | RenderState::STATE_DEPTH_WRITE | RenderState::STATE_DEPTH_TEST);
	RenderManager::Instance()->SetColor(0.2f, 1.0f, 0.2f, 1.0f);
	DebugDrawNode(0, worldBox);
	RenderManager::Instance()->SetState(RenderState::DEFAULT_3D_STATE);
	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void QuadTree::DebugDrawNode(uint32 nodeId, AABBox3 box)
{	
	RenderManager::Instance()->SetColor(0.2f, 0.2f, 1.0f, 1.0f);	
	for (int32 i = 0, size = nodes[nodeId].objects.size(); i<size; ++i)
	{
		RenderHelper::Instance()->DrawBox(nodes[nodeId].objects[i]->GetWorldBoundingBox());
	}
	RenderManager::Instance()->SetColor(0.2f, 1.0f, 0.2f, 1.0f);
	RenderHelper::Instance()->DrawBox(box);
	AABBox3 childBox;
	for (int32 i=0; i<QuadTreeNode::NODE_NONE; ++i)
	{
		if (nodes[nodeId].children[QuadTreeNode::eNodeType(i)]!=INVALID_TREE_NODE_INDEX)
		{
			childBox = box;
			UpdateChildBox(childBox, QuadTreeNode::eNodeType(i));
			childBox.min.z = nodes[nodes[nodeId].children[QuadTreeNode::eNodeType(i)]].zMin;
			childBox.max.z = nodes[nodes[nodeId].children[QuadTreeNode::eNodeType(i)]].zMax;			
			DebugDrawNode(nodes[nodeId].children[QuadTreeNode::eNodeType(i)], childBox);
		}
	}	
	
}

//traverse without recursive calls ... later - it works but might require some rethink
/*void QuadTree::DebugDraw()
{
	Vector<int32> dirStack;
	dirStack.resize(maxTreeDepth+1, 0);
	int currDepth=0;
	AABBox3 currBox = worldBox;
	int currNodeIndex = 0;
	do
	{

		if (dirStack[currDepth] == QuadTreeNode::NODE_NONE)
		{
			if (currDepth == 0)
			{
				break;
			}
			currDepth--;
			UpdateParentBox(currBox, QuadTreeNode::eNodeType(dirStack[currDepth]-1)); //rethink it
			currNodeIndex = nodes[currNodeIndex].parent;
		}
		else if (nodes[currNodeIndex].children[dirStack[currDepth]]!=INVALID_TREE_NODE_INDEX)
		{
			UpdateChildBox(currBox, QuadTreeNode::eNodeType(dirStack[currDepth]));
			currNodeIndex = nodes[currNodeIndex].children[dirStack[currDepth]];
			dirStack[currDepth]++;
			currDepth++;
			dirStack[currDepth] = 0;			
			//visit
			RenderManager::Instance()->SetColor(0.2f, 0.2f, 1.0f, 1.0f);	
			for (int32 i = 0, size = nodes[currNodeIndex].objects.size(); i<size; ++i)
			{
				RenderHelper::Instance()->DrawBox(nodes[currNodeIndex].objects[i]->GetWorldBoundingBox());
			}
			RenderManager::Instance()->SetColor(0.2f, 1.0f, 0.2f, 1.0f);
			RenderHelper::Instance()->DrawBox(currBox);
			//end visit
		}
		else
		{
			dirStack[currDepth]++;
			
		}

	}while(true);

}*/

} //namespace DAVA