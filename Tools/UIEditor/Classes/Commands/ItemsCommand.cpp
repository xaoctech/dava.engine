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
}

void CreateControlCommand::Execute()
{
	HierarchyTreeController::Instance()->CreateNewControl(type, pos);
}
/*
void CreateControlCommand::Rollback()
{
}
*/

DeleteSelectedNodeCommand::DeleteSelectedNodeCommand(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes)
{
	this->nodes = nodes;
}

void DeleteSelectedNodeCommand::Execute()
{
	HierarchyTreeController::Instance()->DeleteNodes(this->nodes);
}

ChangeNodeHeirarchy::ChangeNodeHeirarchy(HierarchyTreeNode* targetNode, HierarchyTreeNode::HIERARCHYTREENODESIDLIST items)
{
	this->targetNode = targetNode;
	this->items = items;
}

void ChangeNodeHeirarchy::Execute()
{
	DVASSERT(targetNode);
	if (!targetNode)
		return;
	
	for (HierarchyTreeNode::HIERARCHYTREENODESIDLIST::iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode((*iter));
		node->SetParent(targetNode);
	}
	
	HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
	ScreenWrapper::Instance()->RequestUpdateView();
}