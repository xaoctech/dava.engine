//
//  UIAggregatorMetadata.cpp
//  UIEditor
//
//  Created by adebt on 3/12/13.
//
//

#include "UIAggregatorMetadata.h"
#include "HierarchyTreeController.h"
#include "HierarchyTreeAggregatorControlNode.h"

using namespace DAVA;

void UIAggregatorMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UIControl* control = this->treeNodeParams[i].GetUIControl();
		
        control->SetName(controlName);
        control->SetPosition(position);
        
        control->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    }
}

QString UIAggregatorMetadata::GetUIControlClassName()
{
	const METADATAPARAMSVECT& params = GetParams();
	if (params.size())
	{
		const BaseMetadataParams& param = params[0];
		HierarchyTreeNode::HIERARCHYTREENODEID id = param.GetTreeNodeID();
		HierarchyTreeAggregatorControlNode* node = dynamic_cast<HierarchyTreeAggregatorControlNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(id));
		if (node)
		{
			const HierarchyTreeAggregatorNode* aggregatorNode = node->GetAggregatorNode();
			if (aggregatorNode)
				return aggregatorNode->GetName();
		}
	}
	
	return UIControlMetadata::GetUIControlClassName();
}