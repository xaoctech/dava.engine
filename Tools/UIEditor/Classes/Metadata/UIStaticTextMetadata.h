//
//  UIStaticTextControlMetadata.h
//  UIEditor
//
//  Created by Yuri Coder on 10/23/12.
//
//

#ifndef __UIEditor__UIStaticTextControlMetadata__
#define __UIEditor__UIStaticTextControlMetadata__

#include "UIControlMetadata.h"
#include "UITextControlMetadata.h"

namespace DAVA {
    
// Metadata class for DAVA UIStaticText control.
class UIStaticTextMetadata : public UITextControlMetadata
{
    Q_OBJECT

public:
    UIStaticTextMetadata(QObject* parent = 0);

protected:
    // Initialize the appropriate control.
    virtual void InitializeControl(const String& controlName, const Vector2& position);
    virtual void UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle);

    virtual QString GetUIControlClassName() { return "UIStaticText"; };
    
    // Set the localized text key.
    virtual void SetLocalizedTextKey(const QString& value);

    // Access to the Static Text.
    UIStaticText* GetActiveStaticText() const;
    
    // Getters/setters.
	virtual int GetAlign();
    virtual void SetAlign(int value);

    virtual Font * GetFont();
    virtual void SetFont(Font* font);
    
    virtual float GetFontSize() const;
    virtual void SetFontSize(float fontSize);
    
    virtual QColor GetFontColor() const;
    virtual void SetFontColor(const QColor& value);
	
	virtual float GetShadowOffsetX() const;
	virtual void SetShadowOffsetX(float offset);
	
	virtual float GetShadowOffsetY() const;
	virtual void SetShadowOffsetY(float offset);
	
	virtual QColor GetShadowColor() const;
	virtual void SetShadowColor(const QColor& value);
};

};

#endif /* defined(__UIEditor__UIStaticTextControlMetadata__) */
