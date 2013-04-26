//
//  UISwitchMetadata.cpp
//  UIEditor
//
//  Created by Denis Bespalov on 3/29/13.
//
//

#include "UISwitchMetadata.h"

namespace DAVA {

UISwitchMetadata::UISwitchMetadata(QObject* parent) :
UIControlMetadata(parent)
{
}

void UISwitchMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
	UIControlMetadata::InitializeControl(controlName, position);
}

void UISwitchMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
	UIControlMetadata::UpdateExtraData(extraData, updateStyle);
}

UISwitch* UISwitchMetadata::GetActiveUISwitch()
{
    return dynamic_cast<UISwitch*>(GetActiveUIControl());
}

bool UISwitchMetadata::GetIsLeftSelected()
{
	UISwitch* activeSwitch = this->GetActiveUISwitch();
	if (!activeSwitch || !VerifyActiveParamID())
	{
		return false;
	}

    return activeSwitch->GetIsLeftSelected();
}

void UISwitchMetadata::SetIsLeftSelected(const bool value)
{
	UISwitch* activeSwitch = this->GetActiveUISwitch();
	if (!activeSwitch || !VerifyActiveParamID())
	{
		return;
	}
    
    activeSwitch->SetIsLeftSelected(value);
}
	
};
