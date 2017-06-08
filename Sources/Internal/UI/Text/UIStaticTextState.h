#pragma once

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"
#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "Render/2D/TextBlock.h"

namespace DAVA
{
class UIControl;
class UIStaticTextComponent;

class UIStaticTextState : public BaseObject
{
public:
    UIStaticTextState(UIControl* control_, UIStaticTextComponent* component_);

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

    void PrepareSprite();
    void ApplyComponentData();

private:
    UIStaticTextState& operator=(const UIStaticTextState&) = delete;

protected:
    ~UIStaticTextState() override;

    UIControl* control;
    UIStaticTextComponent* component;

    TextBlock* textBlock;
    UIControlBackground* textBg;
    UIControlBackground* shadowBg;

};
}
