//
//  UIScrollViewMetadata.h
//  UIEditor
//
//  Created by Denis Bespalov on 4/23/13.
//
//

#ifndef __UIEditor__UIScrollViewMetadata__
#define __UIEditor__UIScrollViewMetadata__

#include "UIControlMetadata.h"
#include "UI/UIScrollView.h"

namespace DAVA {

// Metadata class for DAVA UIList control.
class UIScrollViewMetadata : public UIControlMetadata
{
    Q_OBJECT
	
    // Horizontal position of scroll
    Q_PROPERTY(float HorizontalScrollPosition READ GetHorizontalScrollPosition WRITE SetHorizontalScrollPosition);
    Q_PROPERTY(float VerticalScrollPosition READ GetVerticalScrollPosition WRITE SetVerticalScrollPosition);
	
	
public:
    UIScrollViewMetadata(QObject* parent = 0);

protected:
    // Initialize the appropriate control.
    virtual void InitializeControl(const String& controlName, const Vector2& position);
    virtual void UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle);

    virtual QString GetUIControlClassName() { return "UIScrollView"; };
	
    // Helper to access active UI ScrollView.
    UIScrollView* GetActiveUIScrollView() const;
	
    // Getters/setters.
    float GetHorizontalScrollPosition() const;
	void SetHorizontalScrollPosition(float value);
    float GetVerticalScrollPosition() const;
	void SetVerticalScrollPosition(float value);
};

};

#endif /* defined(__UIEditor__UIScrollViewMetadata__) */
