#pragma once

namespace DAVA
{
class UIControl;
class UITextComponent;
class TextBlock;
class UIControlBackground;

/**
    Internal helper class for text parsing and rendering purpose.    
    Stores component-control pair and helper renderer objects.
    Lifecycle totally managed by system. 
    \sa {UITextComponent, UITextSystem}. 
*/
class UITextSystemLink final
{
public:
    /** Text parser and renderer. */
    inline TextBlock* GetTextBlock() const;
    /** Text layer representation for render. */
    inline UIControlBackground* GetTextBackground() const;
    /** Text shadow layer representation for render. */
    inline UIControlBackground* GetShadowBackground() const;

private:
    UITextSystemLink& operator=(const UITextSystemLink&) = delete;

    UITextSystemLink(UIControl* control_, UITextComponent* component_);
    ~UITextSystemLink();

    /** Apply component properties changes to internal TextBlock and UIControlBackground objects. */
    void ApplyData();

    UIControl* control;
    UITextComponent* component;

    TextBlock* textBlock;
    UIControlBackground* textBg;
    UIControlBackground* shadowBg;

    // Friends
    friend class UITextSystem;
};

TextBlock* UITextSystemLink::GetTextBlock() const
{
    return textBlock;
}

UIControlBackground* UITextSystemLink::GetTextBackground() const
{
    return textBg;
}

UIControlBackground* UITextSystemLink::GetShadowBackground() const
{
    return shadowBg;
}
}