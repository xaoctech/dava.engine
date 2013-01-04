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

CreatePlatformCommand::CreatePlatformCommand(const QString& name, const Vector2& size)
{
	this->name = name;
	this->size = size;
}

void CreatePlatformCommand::Execute()
{
	HierarchyTreeController::Instance()->AddPlatform(name, size);
}


CreateScreenCommand::CreateScreenCommand(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId)
{
	this->name = name;
	this->platformId = platformId;
}

void CreateScreenCommand::Execute()
{
	HierarchyTreeController::Instance()->AddScreen(name, platformId);
}

CreateControlCommand::CreateControlCommand(const QString& type, const QPoint& pos)
{
	this->type = type;
	this->pos = pos;
	this->createdControlID = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
}

void CreateControlCommand::Execute()
{
	HierarchyTreeNode::HIERARCHYTREENODEID newControlID = HierarchyTreeController::Instance()->CreateNewControl(type, pos);
	if (newControlID == HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY)
	{
		// The control wasn't created.
		return;
	}
	
	if (this->createdControlID == HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY)
	{
		// This is the first time the control is created. Remember its ID.
		this->createdControlID = newControlID;
	}
	else
	{
		// The control was already created (and then probably removed and re-created),
		// so we need to update the ID of the control just created with the remembered one.
		// This is needed to do not break Undo/Redo queue.
		HierarchyTreeNode* controlJustCreated = HierarchyTreeController::Instance()->GetTree().GetNode(newControlID);
		if (controlJustCreated)
		{
			// TODO! currently does not work because of crash, found and fix it!
			//controlJustCreated->UpdateId(this->createdControlID);
		}
	}
}

void CreateControlCommand::Rollback()
{
	if (this->createdControlID == HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY)
	{
		// The control wasn't created yet or error happened.
		return;
	}

	HierarchyTreeController::Instance()->DeleteNode(createdControlID);
}

DeleteSelectedNodeCommand::DeleteSelectedNodeCommand(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes)
{
	this->nodes = nodes;
}

void DeleteSelectedNodeCommand::Execute()
{
	HierarchyTreeController::Instance()->DeleteNodes(this->nodes);
}

ChangeNodeHeirarchy::ChangeNodeHeirarchy(HierarchyTreeNode::HIERARCHYTREENODEID targetNodeID, HierarchyTreeNode::HIERARCHYTREENODESIDLIST items)
{
	this->targetNodeID = targetNodeID;
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
		
		// The Previous Parents are stored in the "item ID - parent ID" map.
		this->previousParents.insert(std::make_pair(*iter, parentNode->GetId()));
	}
}

void ChangeNodeHeirarchy::Execute()
{
	HierarchyTreeNode* targetNode = HierarchyTreeController::Instance()->GetTree().GetNode(targetNodeID);
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
			node->SetParent(targetNode);
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
		HierarchyTreeNode* prevParentNode = HierarchyTreeController::Instance()->GetTree().GetNode(iter->second);
		
		if (!currentNode || !prevParentNode)
		{
			continue;
		}
		
		currentNode->SetParent(prevParentNode);
	}
	
	HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
	ScreenWrapper::Instance()->RequestUpdateView();
}
