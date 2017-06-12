#pragma once

namespace DAVA
{
class UIControl;
class UITextComponent;
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

    UITextSystemLink(UIControl* control_, UITextComponent* component_);
    ~UITextSystemLink();

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
