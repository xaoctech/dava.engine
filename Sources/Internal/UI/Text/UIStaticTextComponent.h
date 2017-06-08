#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Render/2D/TextBlock.h"
#include "UI/Components/UIComponent.h"
#include "UI/UIControl.h"

namespace DAVA
{
class UIControl;
class UIStaticTextState;

class UIStaticTextComponent : public UIBaseComponent<UIComponent::STATIC_TEXT_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UIStaticTextComponent, UIBaseComponent<UIComponent::STATIC_TEXT_COMPONENT>);

public:
    enum eTextMultiline
    {
        MULTILINE_DISABLED = 0,
        MULTILINE_ENABLED,
        MULTILINE_ENABLED_BY_SYMBOL
    };
    enum eTextFitting
    {
        FITTING_NONE = 0,
        FITTING_ENLARGE,
        FITTING_REDUCE,
        FITTING_FILL, // ENLARGE | REDUCE
        FITTING_POINTS
    };

protected:
    ~UIStaticTextComponent() override = default;

public:
    UIStaticTextComponent() = default;
    UIStaticTextComponent(const UIStaticTextComponent& src);
    UIStaticTextComponent* Clone() const override;
    UIStaticTextComponent& operator=(const UIStaticTextComponent&) = delete;

    //  Persistent properties:

    void SetAlign(int32 _align);
    int32 GetAlign() const;

    void SetText(const String& text_);
    String GetText() const;

    void SetFitting(eTextFitting fitting_);
    eTextFitting GetFitting() const;

    void SetFontName(const String& fontName_);
    String GetFontName() const;

    void SetColor(const Color& color);
    const Color& GetColor() const;

    void SetMultiline(eTextMultiline multilineType);
    eTextMultiline GetMultiline() const;

    void SetColorInheritType(UIControlBackground::eColorInheritType type);
    UIControlBackground::eColorInheritType GetColorInheritType() const;

    void SetShadowOffset(const Vector2& offset);
    const Vector2& GetShadowOffset() const;

    void SetShadowColor(const Color& color);
    const Color& GetShadowColor() const;

    void SetPerPixelAccuracyType(UIControlBackground::ePerPixelAccuracyType type);
    UIControlBackground::ePerPixelAccuracyType GetPerPixelAccuracyType() const;

    void SetUseRtlAlign(TextBlock::eUseRtlAlign useRtlAlign);
    TextBlock::eUseRtlAlign GetUseRtlAlign() const;

    bool IsForceBiDiSupportEnabled() const;
    void SetForceBiDiSupportEnabled(bool value);

    // Helper properties:

    void SetRequestedTextRectSize(const Vector2& value);
    Vector2 GetRequestedTextRectSize() const;

    void SetModified(bool value);
    bool IsModified() const;

    void SetState(UIStaticTextState*);
    UIStaticTextState* GetState() const;

protected:
    int32 align = eAlign::ALIGN_HCENTER | eAlign::ALIGN_VCENTER;
    String text;
    String fontName;
    eTextMultiline multiline = eTextMultiline::MULTILINE_DISABLED;
    eTextFitting fitting = eTextFitting::FITTING_NONE;
    Color color = Color::White;
    UIControlBackground::eColorInheritType colorInheritType = UIControlBackground::COLOR_IGNORE_PARENT;
    Vector2 shadowOffset = Vector2(0.f, 0.f);
    Color shadowColor = Color::Black;
    UIControlBackground::ePerPixelAccuracyType perPixelAccuracyType = UIControlBackground::PER_PIXEL_ACCURACY_ENABLED;
    TextBlock::eUseRtlAlign useRtlAlign = TextBlock::eUseRtlAlign::RTL_DONT_USE;
    bool forceBiDiSupport = false;

    bool modified = true;

    UIStaticTextState* state;

    Vector2 requestedTextRectSize = Vector2::Zero;

};
}