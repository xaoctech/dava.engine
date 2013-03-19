//
//  UISpinnerMetadata.cpp
//  UIEditor
//
//  Created by Yuri Coder on 3/11/13.
//
//

#include "UISpinnerMetadata.h"

namespace DAVA {

UISpinnerMetadata::UISpinnerMetadata(QObject* parent) :
UIControlMetadata(parent)
{
}

void UISpinnerMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
	UIControlMetadata::InitializeControl(controlName, position);
}

void UISpinnerMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
	UIControlMetadata::UpdateExtraData(extraData, updateStyle);
}

};