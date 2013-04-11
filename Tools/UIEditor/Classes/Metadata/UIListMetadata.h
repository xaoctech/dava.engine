//
//  UIListMetadata.h
//  UIEditor
//
//  Created by Yuri Coder on 3/11/13.
//
//

#ifndef __UIEditor__UIListMetadata__
#define __UIEditor__UIListMetadata__

#include "UIControlMetadata.h"
#include "UI/UIList.h"

namespace DAVA {

// Metadata class for DAVA UIList control.
class UIListMetadata : public UIControlMetadata
{
    Q_OBJECT
	
	Q_PROPERTY(int AggregatorID READ GetAggregatorID WRITE SetAggregatorID);

public:
    UIListMetadata(QObject* parent = 0);

protected:
    // Initialize the appropriate control.
    virtual void InitializeControl(const String& controlName, const Vector2& position);
    virtual void UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle);

    virtual QString GetUIControlClassName() { return "UIList"; };
	
    // Helper to access active UI List.
    UIList* GetActiveUIList() const;
	
	// Properties getters/setters
	int GetAggregatorID();
    void SetAggregatorID(int value);
	
	virtual void SetActiveControlRect(const Rect& rect);
};

};

#endif /* defined(__UIEditor__UIListMetadata__) */
