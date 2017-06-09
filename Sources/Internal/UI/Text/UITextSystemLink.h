#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Render/2D/TextBlock.h"
#include "UI/Components/UIComponent.h"
#include "UI/UIControl.h"

namespace DAVA
{
class UIControl;
class UIStaticTextComponent;

class UITextSystemLink final
{
public:

    inline TextBlock* GetTextBlock() const
    {
        return textBlock;
    }

    inline UIControlBackground* GetTextBackground() const
    {
        return textBg;
    }

    inline UIControlBackground* GetShadowBackground() const
    {
        return shadowBg;
    }


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
}
