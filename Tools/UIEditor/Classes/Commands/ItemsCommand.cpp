//
//  ItemsCommand.cpp
//  UIEditor
//
//  Created by adebt on 10/29/12.
//
//

#include "ItemsCommand.h"
#include "HierarchyTreeController.h"
#include "ScreenWrapper.h"

UndoableHierarchyTreeNodeCommand::UndoableHierarchyTreeNodeCommand()
{
	this->redoNode = NULL;
}

void UndoableHierarchyTreeNodeCommand::SetRedoNode(HierarchyTreeNode* redoNode)
{
	this->redoNode = redoNode;
}

void UndoableHierarchyTreeNodeCommand::ReturnRedoNodeToScene()
{
	if (this->redoNode)
	{
		// Need to recover the node previously deleted.
		HierarchyTreeController::Instance()->ReturnNodeToScene(redoNode);
	}
}

void UndoableHierarchyTreeNodeCommand::PrepareRemoveFromSceneInformation()
{
	if (this->redoNode)
	{
		this->redoNode->PrepareRemoveFromSceneInformation();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CreatePlatformCommand::CreatePlatformCommand(const QString& name, const Vector2& size) :
	UndoableHierarchyTreeNodeCommand()
{
	this->name = name;
	this->size = size;
}

void CreatePlatformCommand::Execute()
{
	if (this->redoNode == NULL)
	{
		SetRedoNode(HierarchyTreeController::Instance()->AddPlatform(name, size));
	}
	else
	{
		ReturnRedoNodeToScene();
	}
}

void CreatePlatformCommand::Rollback()
{
	PrepareRemoveFromSceneInformation();
	
	if (this->redoNode)
	{
		// Remove the created node from the scene, but keep it in memory.
		HierarchyTreeController::Instance()->DeleteNode(this->redoNode->GetId(), false, true);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CreateScreenCommand::CreateScreenCommand(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId) :
	UndoableHierarchyTreeNodeCommand()
{
	this->name = name;
	this->platformId = platformId;
}

void CreateScreenCommand::Execute()
{
	if (this->redoNode == NULL)
	{
		SetRedoNode(HierarchyTreeController::Instance()->AddScreen(name, platformId));
	}
	else
	{
		ReturnRedoNodeToScene();
	}
}

void CreateScreenCommand::Rollback()
{
	PrepareRemoveFromSceneInformation();
	
	if (this->redoNode)
	{
		// Remove the created node from the scene, but keep it in memory.
		HierarchyTreeController::Instance()->DeleteNode(this->redoNode->GetId(), false, true);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CreateAggregatorCommand::CreateAggregatorCommand(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId, const Rect& rect) :
UndoableHierarchyTreeNodeCommand()
{
	this->name = name;
	this->platformId = platformId;
	this->rect = rect;
}

void CreateAggregatorCommand::Execute()
{
	if (this->redoNode == NULL)
	{
		SetRedoNode(HierarchyTreeController::Instance()->AddAggregator(name, platformId, rect));
	}
	else
	{
		ReturnRedoNodeToScene();
	}
}

void CreateAggregatorCommand::Rollback()
{
	PrepareRemoveFromSceneInformation();
	
	if (this->redoNode)
	{
		// Remove the created node from the scene, but keep it in memory.
		HierarchyTreeController::Instance()->DeleteNode(this->redoNode->GetId(), false, true);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CreateControlCommand::CreateControlCommand(const QString& type, const QPoint& pos)
{
	this->type = type;
	this->pos = pos;
	this->createdControlID = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	
	this->redoNode = NULL;
}

void CreateControlCommand::Execute()
{
	if (this->redoNode)
	{
		// Need to recover the node previously deleted.
		HierarchyTreeController::Instance()->ReturnNodeToScene(redoNode);
		return;
	}

	// The command is executed for the first time; create the node.
	HierarchyTreeNode::HIERARCHYTREENODEID newControlID = HierarchyTreeController::Instance()->CreateNewControl(type, pos);
	if (newControlID == HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY)
	{
		// The control wasn't created.
		return;
	}
	
	this->createdControlID = newControlID;
}

void CreateControlCommand::Rollback()
{
	if (this->createdControlID == HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY)
	{
		// The control wasn't created yet or error happened.
		return;
	}

	PrepareRedoInformation();
	
	// OK, we have all the information we need to perform Redo. Remove the created node
	// from the scene, but keep it in memory.
	HierarchyTreeController::Instance()->DeleteNode(createdControlID, false, true);
}

void CreateControlCommand::PrepareRedoInformation()
{
	// Clone the current control node, remember the pointer to the previous node in the list
	// to restore the position of the node removed in case of Redo.
	HierarchyTreeNode* createdNode = HierarchyTreeController::Instance()->GetTree().GetNode(this->createdControlID);
	if (!createdNode || !createdNode->GetParent())
	{
		this->redoNode = NULL;
		return;
	}
	
	createdNode->PrepareRemoveFromSceneInformation();
	this->redoNode = createdNode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

DeleteSelectedNodeCommand::DeleteSelectedNodeCommand(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes)
{
	this->nodes = nodes;
}

void DeleteSelectedNodeCommand::Execute()
{
	if (this->redoNodes.size() == 0)
	{
		// Prepare the Redo information for the first time.
		PrepareRedoInformation();
	}

	// Delete the node from scene, but keep in memory.
	HierarchyTreeController::Instance()->DeleteNodes(this->nodes, false, true);
}

void DeleteSelectedNodeCommand::PrepareRedoInformation()
{
	// Remember the nodes which will be removed from the scene.
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = this->nodes.begin();
		 iter != this->nodes.end(); iter ++)
	{
		HierarchyTreeNode* nodeToRedo = (*iter);
		if (!nodeToRedo || !nodeToRedo->GetParent())
		{
			continue;
		}

		nodeToRedo->PrepareRemoveFromSceneInformation();
		this->redoNodes.push_back(nodeToRedo);
	}
}

void DeleteSelectedNodeCommand::Rollback()
{
	if (redoNodes.size() == 0)
	{
		return;
	}

	HierarchyTreeController::Instance()->ReturnNodeToScene(redoNodes);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

ChangeNodeHeirarchy::ChangeNodeHeirarchy(HierarchyTreeNode::HIERARCHYTREENODEID targetNodeID, HierarchyTreeNode::HIERARCHYTREENODEID afterNodeID, HierarchyTreeNode::HIERARCHYTREENODESIDLIST items)
{
	this->targetNodeID = targetNodeID;
	this->afterNodeID = afterNodeID;
	this->items = items;
	
	// Remember the previous parent IDs for the commands. Note - we cannot store just pointers
	// to the parents since they may gone.
	StorePreviousParents();
}

void ChangeNodeHeirarchy::StorePreviousParents()
{
	for (HierarchyTreeNode::HIERARCHYTREENODESIDLIST::iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode((*iter));
		if (!node)
		{
			continue;
		}
		
		HierarchyTreeNode* parentNode = node->GetParent();
		if (!parentNode)
		{
			continue;
		}
		
		HierarchyTreeNode::HIERARCHYTREENODEID addAfter = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
		HierarchyTreeNode::HIERARCHYTREENODEID lastId = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
		const HierarchyTreeNode::HIERARCHYTREENODESLIST& childs = parentNode->GetChildNodes();
		for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator citer = childs.begin();
			 citer != childs.end();
			 ++citer)
		{
			if (node == (*citer))
				addAfter = lastId;
			lastId = (*citer)->GetId();
		}
		
		// The Previous Parents are stored in the "item ID - parent ID" map.
		this->previousParents.insert(std::make_pair(*iter, PreviousState(parentNode->GetId(), addAfter)));
	}
}

void ChangeNodeHeirarchy::Execute()
{
	HierarchyTreeNode* targetNode = HierarchyTreeController::Instance()->GetTree().GetNode(targetNodeID);
	HierarchyTreeNode* insertAfterNode = HierarchyTreeController::Instance()->GetTree().GetNode(afterNodeID);
	if (!targetNode)
	{
		// Possible in Redo case if some changes in tree were made.
		return;
	}

	
	for (HierarchyTreeNode::HIERARCHYTREENODESIDLIST::iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode((*iter));
		if (node)
		{
			//YZ backlight parent rect
			bool isNodeSelected = false;
			HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
			if (controlNode)
			{
				isNodeSelected = HierarchyTreeController::Instance()->IsControlSelected(controlNode);
				HierarchyTreeController::Instance()->UnselectControl(controlNode);
			}

			node->SetParent(targetNode, insertAfterNode);
			//insertAfterNode = node;
			
			if (isNodeSelected)
			{
				HierarchyTreeController::Instance()->SelectControl(controlNode);
			}
		}
	}
	
	HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
	ScreenWrapper::Instance()->RequestUpdateView();
}

void ChangeNodeHeirarchy::Rollback()
{
	for (PARENTNODESMAPITER iter = previousParents.begin(); iter != previousParents.end(); iter ++)
	{
		HierarchyTreeNode* currentNode = HierarchyTreeController::Instance()->GetTree().GetNode(iter->first);
		HierarchyTreeNode* prevParentNode = HierarchyTreeController::Instance()->GetTree().GetNode(iter->second.parent);
		HierarchyTreeNode* prevAddedAfter = HierarchyTreeController::Instance()->GetTree().GetNode(iter->second.addedAfter);
		
		if (!currentNode || !prevParentNode)
		{
			continue;
		}
		
		currentNode->SetParent(prevParentNode, prevAddedAfter);
	}
	
	HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
	ScreenWrapper::Instance()->RequestUpdateView();
}
