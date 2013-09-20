
#include "Render/Highlevel/SpatialTree.h"

namespace DAVA
{

QuadTree::QuadTree(const AABBox3& _worldBox, int32 _maxTreeDepth): worldBox(_worldBox), maxTreeDepth(_maxTreeDepth)
{	
	QuadTreeNode root;
	root.parent=INVALID_TREE_NODE_INDEX;
	for (int i=0; i<4; i++)
		root.children[i] = INVALID_TREE_NODE_INDEX;
	nodes.push_back(root);
}

bool QuadTree::CheckBoxIntersectBranch(const AABBox3& objBox, float32 xmin, float32 ymin, float32 xmax, float32 ymax)
{
	
	if ((xmin>objBox.max.x)||(ymin>objBox.max.y)||(xmax<objBox.min.x)||(ymax<objBox.min.y))
		return false;
	return true;
}

void QuadTree::AddObject(RenderObject *object)
{
	DVASSERT(object->GetTreeNodeIndex()==INVALID_TREE_NODE_INDEX);
	uint32 currIndex = 0;
	AABBox3 currBox = worldBox;
	const AABBox3& objBox = object->GetWorldBoundingBox();
	if (!worldBox.IsInside(objBox))
	{
		//object is somehow outside the world just add to root
		nodes[0].objects.push_back(object);
		object->SetTreeNodeIndex(0);
		return;
	}

	
	
	for (int32 depth=0; depth<maxTreeDepth; depth++) //assure we don't run deeper than maxTreeLength
	{
		QuadTreeNode::eNodeType fitNode = QuadTreeNode::NODE_NONE;		
		if (CheckBoxIntersectBranch(objBox, currBox.min.x, currBox.min.y, (currBox.min.x+currBox.max.x)/2, (currBox.min.y+currBox.max.y)/2))//LB
		{
			fitNode = QuadTreeNode::NODE_LB;
		}
		if (CheckBoxIntersectBranch(objBox, (currBox.min.x+currBox.max.x)/2, currBox.min.y, currBox.max.x, (currBox.min.y+currBox.max.y)/2))//RB
		{
			if (fitNode!=QuadTreeNode::NODE_NONE)
				break; //fit more than one - add here
			fitNode = QuadTreeNode::NODE_RB;
		}
		if (CheckBoxIntersectBranch(objBox, currBox.min.x, (currBox.min.y+currBox.max.y)/2, (currBox.min.x+currBox.max.x)/2, currBox.max.y))//LT
		{
			if (fitNode!=QuadTreeNode::NODE_NONE)
				break; //fit more than one - add here
			fitNode = QuadTreeNode::NODE_LT;
		}
		if (CheckBoxIntersectBranch(objBox, (currBox.min.x+currBox.max.x)/2, (currBox.min.y+currBox.max.y)/2, currBox.max.x, currBox.max.y))//RT
		{
			if (fitNode!=QuadTreeNode::NODE_NONE)
				break; //fit more than one - add here
			fitNode = QuadTreeNode::NODE_RT;
		}				
		//create corresponding node if not exist
		if (nodes[currIndex].children[fitNode]==INVALID_TREE_NODE_INDEX)
		{
			int newNodeIndex;
			if (emptyNodes.size()) //take from empty
			{
				newNodeIndex = emptyNodes.back();
				emptyNodes.pop_back();
			}
			else //ore create new node
			{
				newNodeIndex = nodes.size();
				nodes.resize(newNodeIndex+1);				
			}
			nodes[newNodeIndex].parent = currIndex;
			for (int32 i = 0; i<4; i++)
				nodes[newNodeIndex].children[i] = INVALID_TREE_NODE_INDEX;
			nodes[currIndex].children[fitNode] = newNodeIndex;
		}
		//as object completely fits node - continue down the tree 
		currIndex = nodes[currIndex].children[fitNode];
	}
	//node doesn't fit any sub node completely or max depth reached
	nodes[currIndex].objects.push_back(object);
	object->SetTreeNodeIndex(currIndex);

}

void QuadTree::RemoveObject(RenderObject *object)
{
	uint32 currIndex = object->GetTreeNodeIndex();
	DVASSERT(currIndex!=INVALID_TREE_NODE_INDEX);
	object->SetTreeNodeIndex(INVALID_TREE_NODE_INDEX);
	List<RenderObject*>::iterator it = std::find(nodes[currIndex].objects.begin(), nodes[currIndex].objects.end(), object);
	DVASSERT(it!=nodes[currIndex].objects.end());
	nodes[currIndex].objects.erase(it);
	CheckRemoveBranch(currIndex);
}

void QuadTree::CheckRemoveBranch(uint32 nodeIndex)
{
	while (nodeIndex!=0) //do not remove root anyway 
	{
		QuadTreeNode& currNode = nodes[nodeIndex];
		if (!currNode.objects.empty()) 
			return; //dont remove non-empty node
		
		if ((currNode.children[0]!=INVALID_TREE_NODE_INDEX)||
			(currNode.children[1]!=INVALID_TREE_NODE_INDEX)||
			(currNode.children[2]!=INVALID_TREE_NODE_INDEX)||
			(currNode.children[3]!=INVALID_TREE_NODE_INDEX))
			return; //dont remove node with children;

		//node is empty and has no children - remove it from tree structure and validate it's parent
		emptyNodes.push_back(nodeIndex);
		for (int32 i=0; i<4; i++)
			if (nodes[currNode.parent].children[i]==nodeIndex)
				nodes[currNode.parent].children[i]=INVALID_TREE_NODE_INDEX;
		nodeIndex = currNode.parent;
	}
}

void QuadTree::ObjectUpdated(RenderObject *object)
{
	//return back here once through with everything else
	uint32 currIndex = object->GetTreeNodeIndex();
	DVASSERT(currIndex!=INVALID_TREE_NODE_INDEX);	
	//*for now just simple remove and add*//
	object->SetTreeNodeIndex(INVALID_TREE_NODE_INDEX);
	List<RenderObject*>::iterator it = std::find(nodes[currIndex].objects.begin(), nodes[currIndex].objects.end(), object);
	DVASSERT(it!=nodes[currIndex].objects.end());
	nodes[currIndex].objects.erase(it);
	AddObject(object);	
	if (object->GetTreeNodeIndex()!=currIndex)
		CheckRemoveBranch(currIndex);
	
}
void QuadTree::ProcessClipping(Frustum *frustum)
{

}
void QuadTree::UpdateTree()
{
	//nothing special for QuadTree
}

void QuadTree::DebugDraw()
{
	Vector<int32> dirStack;
	dirStack.resize(maxTreeDepth+1, 0);
	int currDepth=0;
	int currNodeIndex = 0;
	RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
	RenderManager::Instance()->SetState(RenderState::STATE_COLORMASK_ALL | RenderState::STATE_DEPTH_WRITE | RenderState::STATE_DEPTH_TEST);
	RenderManager::Instance()->SetColor(0.2f, 1.0f, 0.2f, 1.0f);
	do
	{
		if (nodes[currNodeIndex].children[dirStack[currDepth]]!=INVALID_TREE_NODE_INDEX)
		{
			//nodes[currNodeIndex].children[dirStack[currDepth]]
		}

	}while(true);


	//restore
	RenderManager::Instance()->SetState(RenderState::DEFAULT_3D_STATE);
	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

}

} //namespace DAVA