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

class UIStaticTextDrawer : public BaseObject
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

    UIStaticTextDrawer(UIControl* control_, UIStaticTextComponent* component_);

    Rect CalculateTextBlockRect(const UIGeometricData& geometricData) const;
    void Draw(const UIGeometricData& geometricData);

private:
    UIStaticTextDrawer& operator=(const UIStaticTextDrawer&) = delete;

    void PrepareSprite();
    void applyModifications();

protected:
    ~UIStaticTextDrawer() override = default;

    // Rect CalculateTextBlockRect(const UIGeometricData& geometricData) const;

    RefPtr<UIControl> control;
    RefPtr<UIStaticTextComponent> component;

    RefPtr<TextBlock> textBlock;
    RefPtr<UIControlBackground> textBg;
    RefPtr<UIControlBackground> shadowBg;

//inline TextBlock* GetTextBlock()
//{
//    return textBlock;
//}

//inline UIControlBackground* GetTextBackground() const
//{
//    return textBg;
//};
//inline UIControlBackground* GetShadowBackground() const
//{
//    return shadowBg;
//};


#if defined(LOCALIZATION_DEBUG)
    void DrawLocalizationDebug(const UIGeometricData& textGeomData) const;
    void DrawLocalizationErrors(const UIGeometricData& textGeomData, const UIGeometricData& elementGeomData) const;
    void RecalculateDebugColoring();

    DebugHighliteColor warningColor;
    DebugHighliteColor lineBreakError;
#endif
};
}
