//
//  BaseMetadataParams.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/24/12.
//
//

#include "BaseMetadataParams.h"
using namespace DAVA;

BaseMetadataParams::BaseMetadataParams(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID, UIControl* uiControl)
{
    this->treeNodeID = treeNodeID;
    this->uiControl = uiControl;
}

HierarchyTreeNode::HIERARCHYTREENODEID BaseMetadataParams::GetTreeNodeID() const
{
    return this->treeNodeID;
}

UIControl* BaseMetadataParams::GetUIControl() const
{
    return this->uiControl;
}
