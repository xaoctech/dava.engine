#pragma once

namespace DAVA
{
class UIControl;
class UIStaticTextComponent;
class TextBlock;
class UIControlBackground;

class UITextSystemLink final
{
public:
    inline TextBlock* GetTextBlock() const;
    inline UIControlBackground* GetTextBackground() const;
    inline UIControlBackground* GetShadowBackground() const;

private:
    UITextSystemLink& operator=(const UITextSystemLink&) = delete;

    UITextSystemLink(UIControl* control_, UIStaticTextComponent* component_);
    ~UITextSystemLink();

    void ApplyData();

    UIControl* control;
    UIStaticTextComponent* component;

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
