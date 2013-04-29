//
//  UIScrollViewMetadata.cpp
//  UIEditor
//
//  Created by Denis Bespalov on 4/23/13.
//
//

#include "UIScrollViewMetadata.h"

namespace DAVA {

UIScrollViewMetadata::UIScrollViewMetadata(QObject* parent) :
	UIControlMetadata(parent)
{
}

UIScrollView* UIScrollViewMetadata::GetActiveUIScrollView() const
{
	return dynamic_cast<UIScrollView*>(GetActiveUIControl());
}

void UIScrollViewMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
	UIControlMetadata::InitializeControl(controlName, position);
	
	int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
		// Initialize UIList
        UIScrollView* scrollView = dynamic_cast<UIScrollView*>(this->treeNodeParams[i].GetUIControl());
		if (scrollView)
		{
			scrollView->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
		}
    }	
}

void UIScrollViewMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
	UIControlMetadata::UpdateExtraData(extraData, updateStyle);
}


float UIScrollViewMetadata::GetHorizontalScrollPosition() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
	Vector2 offset = GetActiveUIScrollView()->GetOffset();
	
	return (-1) * qRound(offset.x);
}

void UIScrollViewMetadata::SetHorizontalScrollPosition(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	Vector2 offset = GetActiveUIScrollView()->GetOffset();
	offset.x = (-1) * value;	

	GetActiveUIScrollView()->SetOffset(offset);
}


float UIScrollViewMetadata::GetVerticalScrollPosition() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	Vector2 offset = GetActiveUIScrollView()->GetOffset();
	
	return (-1) * qRound(offset.y);
}

void UIScrollViewMetadata::SetVerticalScrollPosition(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	Vector2 offset = GetActiveUIScrollView()->GetOffset();
	offset.y = (-1) * value;

	GetActiveUIScrollView()->SetOffset(offset);
}

}