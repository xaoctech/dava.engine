//
//  ButtonNodeMetadata.h
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#ifndef __UIEditor__ButtonNodeMetadata__
#define __UIEditor__ButtonNodeMetadata__

#include "UITextControlMetadata.h"
#include "UI/UIButton.h"

namespace DAVA {
    
// Metadata class for DAVA UIButton control.
class UIButtonMetadata : public UITextControlMetadata
{
    Q_OBJECT

public:
    UIButtonMetadata(QObject* parent = 0);    

protected:
    // Initialization.
    virtual void InitializeControl(const String& controlName, const Vector2& position);
    virtual void UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle);
    
    virtual QString GetUIControlClassName() { return "UIButton"; };

    // Set the text of the button.
    virtual void SetLocalizedTextKey(const QString& value);

    // Helper to access active UI Button.
    UIButton* GetActiveUIButton() const;
 
    virtual float GetFontSize() const;
    virtual void SetFontSize(float fontSize);
    
    // Color getter/setter. Also virtual.
    virtual Font * GetFont();
    virtual void SetFont(Font* font);
    
    virtual QColor GetFontColor() const;
    virtual void SetFontColor(const QColor& value);
	
	// Shadow offset and color getters/setters
	virtual float GetShadowOffsetX() const;
	virtual void SetShadowOffsetX(float offset);
	
	virtual float GetShadowOffsetY() const;
	virtual void SetShadowOffsetY(float offset);
	
	virtual QColor GetShadowColor() const;
	virtual void SetShadowColor(const QColor& value);

    // Color getter/setter. Also virtual.
    virtual QColor GetColor();
    virtual void SetColor(const QColor& value);

    // Sprite getter/setter.
    virtual void SetSprite(const QString& value);
    virtual QString GetSprite();
    
    virtual void SetSpriteFrame(int value);
    virtual int GetSpriteFrame();
    
    // Drawing flags getters/setters.
    virtual int GetDrawType();
    virtual void SetDrawType(int value);

    virtual int GetColorInheritType();
    virtual void SetColorInheritType(int value);

    virtual int GetAlign();
    virtual void SetAlign(int value);
    
    // For UI Button localized text depends on state, so overriding this function.
    virtual UIControl::eControlState GetCurrentStateForLocalizedText() const;
    
    // Propery Dirty flags handling.
    // Localized Text.
    void UpdatePropertyDirtyFlagForLocalizedText();
    
    // Font property for selected state
    Font * GetFontForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForFont();

    // Font Size.
    float GetFontSizeForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForFontSize();

    // Font Color.
    QColor GetFontColorForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForFontColor();

    // Color.
    QColor GetColorForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForColor();

    // Sprite Name.
    QString GetSpriteNameForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForSpriteName();
    
    // Sprite Frame.
    int GetSpriteFrameForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForSpriteFrame();
    
    // Draw Type.
    void UpdatePropertyDirtyFlagForDrawType();
  
    // Color Inherit Type.
    int GetColorInheritTypeForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForColorInheritType();

    // Align Type.
    int GetAlignForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForAlign();
    
    // Recover dirty flags.
    void RecoverPropertyDirtyFlags();
};

};

#endif /* defined(__UIEditor__ButtonNodeMetadata__) */
