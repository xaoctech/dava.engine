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
    TextBlock* GetTextBlock() const;
    /** Text layer representation for render. */
    UIControlBackground* GetTextBackground() const;
    /** Text shadow layer representation for render. */
    UIControlBackground* GetShadowBackground() const;

private:
    UITextSystemLink& operator=(const UITextSystemLink&) = delete;

    UITextSystemLink(UIControl* control_, UITextComponent* component_);
    ~UITextSystemLink() = default;

    /** Apply component properties changes to internal TextBlock and UIControlBackground objects. */
    void ApplyData();

    UIControl* control;
    UITextComponent* component;

    RefPtr<TextBlock> textBlock;
    RefPtr<UIControlBackground> textBg;
    RefPtr<UIControlBackground> shadowBg;

    // Friends
    friend class UITextSystem;
};

inline TextBlock* UITextSystemLink::GetTextBlock() const
{
    return textBlock.Get();
}

inline UIControlBackground* UITextSystemLink::GetTextBackground() const
{
    return textBg.Get();
}

inline UIControlBackground* UITextSystemLink::GetShadowBackground() const
{
    return shadowBg.Get();
}
}