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

class UITextSystemLink : public BaseObject
{
public:
    UITextSystemLink(UIControl* control_, UIStaticTextComponent* component_);

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

    void ApplyData();

private:
    UITextSystemLink& operator=(const UITextSystemLink&) = delete;

protected:
    ~UITextSystemLink() override;

    UIControl* control;
    UIStaticTextComponent* component;

    TextBlock* textBlock;
    UIControlBackground* textBg;
    UIControlBackground* shadowBg;

    friend class UITextSystem;
};
}
