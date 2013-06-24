//
//  HierarchyTreeAggregatorControlNode.cpp
//  UIEditor
//
//  Created by adebt on 3/13/13.
//
//

#include "HierarchyTreeAggregatorControlNode.h"
#include "HierarchyTreeAggregatorNode.h"

using namespace DAVA;

HierarchyTreeAggregatorControlNode::HierarchyTreeAggregatorControlNode(HierarchyTreeAggregatorNode* parentAggregator,
																	   HierarchyTreeNode* parent,
																	   UIControl* uiObject,
																	   const QString& name):
	HierarchyTreeControlNode(parent, uiObject, name),
	parentAggregatorSave(NULL)
{
	this->parentAggregator = parentAggregator;
}

HierarchyTreeAggregatorControlNode::HierarchyTreeAggregatorControlNode(HierarchyTreeNode* parent, const HierarchyTreeAggregatorControlNode* node) :
	HierarchyTreeControlNode(parent, node),
	parentAggregatorSave(NULL)
{
	parentAggregator = node->parentAggregator;
	
	if (parent && parentAggregator)
		parentAggregator->AddChild(this);
}

HierarchyTreeAggregatorControlNode::~HierarchyTreeAggregatorControlNode()
{
	if (parentAggregator)
		parentAggregator->RemoveChild(this);
}

void HierarchyTreeAggregatorControlNode::SetAggregatorNode(HierarchyTreeAggregatorNode* parentAggregator)
{
	if (this->parentAggregator)
		this->parentAggregator->RemoveChild(this);
	
	this->parentAggregator = parentAggregator;
	if (this->parentAggregator)
		this->parentAggregator->AddChild(this);
}

void HierarchyTreeAggregatorControlNode::RemoveTreeNodeFromScene()
{
	parentAggregatorSave = parentAggregator;
	SetAggregatorNode(NULL);
	HierarchyTreeControlNode::RemoveTreeNodeFromScene();
}

void HierarchyTreeAggregatorControlNode::ReturnTreeNodeToScene()
{
	SetAggregatorNode(parentAggregatorSave);
	HierarchyTreeControlNode::ReturnTreeNodeToScene();
}

FilePath HierarchyTreeAggregatorControlNode::GetAggregatorPath() const
{
	UIAggregatorControl* aggregatorControl = dynamic_cast<UIAggregatorControl*>(GetUIObject());
	if (aggregatorControl)
	{
		return aggregatorControl->GetAggregatorPath();
	}

	return FilePath();
}
