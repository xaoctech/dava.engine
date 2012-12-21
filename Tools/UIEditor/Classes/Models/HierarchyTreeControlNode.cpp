//
//  HierarchyTreeControlNode.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#include "HierarchyTreeControlNode.h"

HierarchyTreeControlNode::HierarchyTreeControlNode(HierarchyTreeNode* parent,
												   UIControl* uiObject,
												   const QString& name) :
	HierarchyTreeNode(name)
{
	this->parent = parent;
	this->uiObject = uiObject;
	
	AddControlToParent();
}

HierarchyTreeControlNode::HierarchyTreeControlNode(HierarchyTreeNode* parent,
												   const HierarchyTreeControlNode* node):
	HierarchyTreeNode(node)
{
	this->parent = parent;
	this->uiObject = node->GetUIObject()->Clone();
	
	//remove real child
	const List<UIControl* > &realChildren = GetUIObject()->GetRealChildren();
	for (List<UIControl* >::const_iterator iter = realChildren.begin();
		 iter != realChildren.end();
		 ++iter)
	{
        GetUIObject()->RemoveControl(*iter);
	}
	
	AddControlToParent();
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& child = node->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = child.begin();
		 iter != child.end();
		 ++iter)
	{
		const HierarchyTreeControlNode* controlNode = dynamic_cast<const HierarchyTreeControlNode*>((*iter));
		if (!controlNode)
			continue;
				
		AddTreeNode(new HierarchyTreeControlNode(this, controlNode));
	}
}

void HierarchyTreeControlNode::AddControlToParent()
{
	if (!parent)
		return;
	
	DVASSERT(uiObject);
	
	HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(parent);
	if (screenNode)
	{
		screenNode->GetScreen()->AddControl(this->uiObject);
	}
	else
	{
		//HierarchyTreeControlNode* controlNode = GetControlNode();
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(parent);
		if (controlNode)
		{
			UIControl* control = controlNode->GetUIObject();
			if (control)
				control->AddControl(this->uiObject);
		}
	}	
}

HierarchyTreeControlNode::~HierarchyTreeControlNode()
{
	Cleanup();
	
	UIControl* parent = uiObject->GetParent();
	if (parent)
		parent->RemoveControl(uiObject);
	else
		SafeRelease(uiObject);
}

HierarchyTreeScreenNode* HierarchyTreeControlNode::GetScreenNode() const
{
	if (!parent)
		return NULL;
	
	HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(parent);
	if (screenNode)
		return screenNode;
	
	HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(parent);
	if (controlNode)
		return controlNode->GetScreenNode();
	
	return NULL;
}

HierarchyTreeControlNode* HierarchyTreeControlNode::GetControlNode() const
{
	HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(parent);
	if (controlNode)
		return controlNode;
	
	return NULL;
}

void HierarchyTreeControlNode::SetParent(HierarchyTreeNode* node)
{
	if (node == parent)
		return;
	
	HierarchyTreeControlNode* newParentControl = dynamic_cast<HierarchyTreeControlNode* >(node);
	HierarchyTreeScreenNode* newParentScreen = dynamic_cast<HierarchyTreeScreenNode* >(node);
	DVASSERT(newParentControl || newParentScreen);
	if (!newParentControl && !newParentScreen)
		return;
	
	if (newParentControl)
	{
		newParentControl->AddTreeNode(this);
		if (uiObject && newParentControl->GetUIObject())
			newParentControl->GetUIObject()->AddControl(uiObject);
	}
	else if (newParentScreen)
	{
		newParentScreen->AddTreeNode(this);
		if (uiObject && newParentScreen->GetScreen())
			newParentScreen->GetScreen()->AddControl(uiObject);
	}
	
	if (parent)
	{
		parent->RemoveTreeNode(this, false);
	}
	
	parent = node;
}

Vector2 HierarchyTreeControlNode::GetParentDelta(bool skipControl/* = false*/) const
{
	Vector2 parentDelta(0, 0);
	
	if (!skipControl && uiObject)
	{
		Rect rect = uiObject->GetRect();
		parentDelta = rect.GetPosition();
	}
	
	const HierarchyTreeControlNode* parentControl = dynamic_cast<const HierarchyTreeControlNode* >(parent);
	if (parentControl)
		parentDelta += parentControl->GetParentDelta();
	return parentDelta;
}