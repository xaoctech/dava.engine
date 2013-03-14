//
//  UIListMetadata.cpp
//  UIEditor
//
//  Created by Yuri Coder on 3/11/13.
//
//

#include "UIListMetadata.h"
namespace DAVA {

UIListMetadata::UIListMetadata(QObject* parent) :
	UIControlMetadata(parent)
{
}

void UIListMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
	UIControlMetadata::InitializeControl(controlName, position);
}

void UIListMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
	UIControlMetadata::UpdateExtraData(extraData, updateStyle);
}

};