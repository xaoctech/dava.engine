//
//  UISwitchMetadata.h
//  UIEditor
//
//  Created by Denis Bespalov on 3/29/13.
//
//

#ifndef __UIEditor__UISwitchMetadata__
#define __UIEditor__UISwitchMetadata__

#include "UIControlMetadata.h"
#include "UI/UISwitch.h"

namespace DAVA {
	
// Metadata class for DAVA UISwitch control.
class UISwitchMetadata : public UIControlMetadata
{
	Q_OBJECT
	
    Q_PROPERTY(bool IsLeftSelected READ GetIsLeftSelected WRITE SetIsLeftSelected);

public:
	UISwitchMetadata(QObject* parent = 0);

protected:
	// Initialize the appropriate control.
	virtual void InitializeControl(const String& controlName, const Vector2& position);
	virtual void UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle);

	virtual QString GetUIControlClassName() { return "UISwitch"; };
	
	bool GetIsLeftSelected();
    void SetIsLeftSelected(const bool value);
	
	// Helper methods.
	UISwitch* GetActiveUISwitch();
};

}

#endif /* defined(__UIEditor__UISwitchMetadata__) */
