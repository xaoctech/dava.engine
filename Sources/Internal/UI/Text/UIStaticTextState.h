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

#if defined(LOCALIZATION_DEBUG)
    static const Color HIGHLIGHT_COLORS[];
    enum DebugHighliteColor
    {
        RED = 0,
        BLUE,
        YELLOW,
        WHITE,
        MAGENTA,
        GREEN,
        NONE
    };
    static const float32 LOCALIZATION_RESERVED_PORTION;
#endif

    UIStaticTextState(UIControl* control_, UIStaticTextComponent* component_);

    void Draw(const UIGeometricData& geometricData, const Color& parentColor);

private:
    UIStaticTextState& operator=(const UIStaticTextState&) = delete;

    void PrepareSprite();
    void ApplyComponentData();

protected:
    ~UIStaticTextState() override;

    RefPtr<UIControl> control;
    RefPtr<UIStaticTextComponent> component;

    TextBlock* textBlock;
    UIControlBackground* textBg;
    UIControlBackground* shadowBg;

#if defined(LOCALIZATION_DEBUG)
    void DrawLocalizationDebug(const UIGeometricData& textGeomData) const;
    void DrawLocalizationErrors(const UIGeometricData& textGeomData) const;
#endif
};
}
