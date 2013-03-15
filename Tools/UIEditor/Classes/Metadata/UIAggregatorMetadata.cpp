//
//  UIAggregatorMetadata.cpp
//  UIEditor
//
//  Created by adebt on 3/12/13.
//
//

#include "UIAggregatorMetadata.h"

void UIAggregatorMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UIControl* control = this->treeNodeParams[i].GetUIControl();
		
        control->SetName(controlName);
        //control->SetSize(INITIAL_CONTROL_SIZE);
        control->SetPosition(position);
        
        control->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    }
}