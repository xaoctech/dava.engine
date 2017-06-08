#include "UIStaticTextComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UIStaticTextState.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIStaticTextComponent)
{
    ReflectionRegistrator<UIStaticTextComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIStaticTextComponent* o) { o->Release(); })
    .Field("align", &UIStaticTextComponent::GetAlign, &UIStaticTextComponent::SetAlign)[M::FlagsT<eAlign>()]
    .Field("color", &UIStaticTextComponent::GetColor, &UIStaticTextComponent::SetColor)
    .Field("colorInheritType", &UIStaticTextComponent::GetColorInheritType, &UIStaticTextComponent::SetColorInheritType)[M::EnumT<UIControlBackground::eColorInheritType>()]
    .Field("fitting", &UIStaticTextComponent::GetFitting, &UIStaticTextComponent::SetFitting)[M::EnumT<eTextFitting>()]
    .Field("fontName", &UIStaticTextComponent::GetFontName, &UIStaticTextComponent::SetFontName)
    .Field("forceBiDiSupport", &UIStaticTextComponent::IsForceBiDiSupportEnabled, &UIStaticTextComponent::SetForceBiDiSupportEnabled)
    .Field("multiline", &UIStaticTextComponent::GetMultiline, &UIStaticTextComponent::SetMultiline)[M::EnumT<eTextMultiline>()]
    .Field("perPixelAccuracyType", &UIStaticTextComponent::GetPerPixelAccuracyType, &UIStaticTextComponent::SetPerPixelAccuracyType)[M::EnumT<UIControlBackground::ePerPixelAccuracyType>()]
    .Field("shadowColor", &UIStaticTextComponent::GetShadowColor, &UIStaticTextComponent::SetShadowColor)
    .Field("shadowOffset", &UIStaticTextComponent::GetShadowOffset, &UIStaticTextComponent::SetShadowOffset)
    .Field("text", &UIStaticTextComponent::GetText, &UIStaticTextComponent::SetText)
    .Field("useRtlAlign", &UIStaticTextComponent::GetUseRtlAlign, &UIStaticTextComponent::SetUseRtlAlign)[M::EnumT<TextBlock::eUseRtlAlign>()]
    .End();
}

UIStaticTextComponent::UIStaticTextComponent(const UIStaticTextComponent& src)
    : UIBaseComponent(src)
    , align(src.align)
    , text(src.text)
    , fontName(src.fontName)
    , multiline(src.multiline)
    , fitting(src.fitting)
    , color(src.color)
    , colorInheritType(src.colorInheritType)
    , shadowOffset(src.shadowOffset)
    , shadowColor(src.shadowColor)
    , perPixelAccuracyType(src.perPixelAccuracyType)
    , useRtlAlign(src.useRtlAlign)
    , forceBiDiSupport(src.forceBiDiSupport)
    , modified(true)
{
}

UIStaticTextComponent* UIStaticTextComponent::Clone() const
{
    return new UIStaticTextComponent(*this);
}

void UIStaticTextComponent::SetAlign(int32 value)
{
    if (align != value)
    {
        align = value;
        modified = true;
    }
}

int32 UIStaticTextComponent::GetAlign() const
{
    return align;
}

void UIStaticTextComponent::SetText(const String& value)
{
    if (text != value)
    {
        text = value;
        modified = true;
    }
}

String UIStaticTextComponent::GetText() const
{
    return text;
}

void UIStaticTextComponent::SetFitting(eTextFitting value)
{
    if (fitting != value)
    {
        fitting = value;
        modified = true;
    }
}

UIStaticTextComponent::eTextFitting UIStaticTextComponent::GetFitting() const
{
    return fitting;
}

void UIStaticTextComponent::SetFontName(const String& value)
{
    if (fontName != value)
    {
        fontName = value;
        modified = true;
    }
}

String UIStaticTextComponent::GetFontName() const
{
    return fontName;
}

void UIStaticTextComponent::SetColor(const Color& value)
{
    if (color != value)
    {
        color = value;
        modified = true;
    }
}

const Color& UIStaticTextComponent::GetColor() const
{
    return color;
}

void UIStaticTextComponent::SetMultiline(eTextMultiline value)
{
    if (multiline != value)
    {
        multiline = value;
        modified = true;
    }
}

UIStaticTextComponent::eTextMultiline UIStaticTextComponent::GetMultiline() const
{
    return multiline;
}

void UIStaticTextComponent::SetColorInheritType(UIControlBackground::eColorInheritType value)
{
    if (colorInheritType != value)
    {
        colorInheritType = value;
        modified = true;
    }
}

UIControlBackground::eColorInheritType UIStaticTextComponent::GetColorInheritType() const
{
    return colorInheritType;
}

void UIStaticTextComponent::SetShadowOffset(const Vector2& value)
{
    if (shadowOffset != value)
    {
        shadowOffset = value;
        modified = true;
    }
}

const Vector2& UIStaticTextComponent::GetShadowOffset() const
{
    return shadowOffset;
}

void UIStaticTextComponent::SetShadowColor(const Color& value)
{
    if (shadowColor != value)
    {
        shadowColor = value;
        modified = true;
    }
}

const Color& UIStaticTextComponent::GetShadowColor() const
{
    return shadowColor;
}

void UIStaticTextComponent::SetPerPixelAccuracyType(UIControlBackground::ePerPixelAccuracyType value)
{
    if (perPixelAccuracyType != value)
    {
        perPixelAccuracyType = value;
        modified = true;
    }
}

UIControlBackground::ePerPixelAccuracyType UIStaticTextComponent::GetPerPixelAccuracyType() const
{
    return perPixelAccuracyType;
}

void UIStaticTextComponent::SetUseRtlAlign(TextBlock::eUseRtlAlign value)
{
    if (useRtlAlign != value)
    {
        useRtlAlign = value;
        modified = true;
    }
}

TextBlock::eUseRtlAlign UIStaticTextComponent::GetUseRtlAlign() const
{
    return useRtlAlign;
}

void UIStaticTextComponent::SetForceBiDiSupportEnabled(bool value)
{
    if (forceBiDiSupport != value)
    {
        forceBiDiSupport = value;
        modified = true;
    }
}

void UIStaticTextComponent::SetRequestedTextRectSize(const Vector2& value)
{
    if (requestedTextRectSize != value)
    {
        requestedTextRectSize = value;
        modified = true;
    }
}

DAVA::Vector2 UIStaticTextComponent::GetRequestedTextRectSize() const
{
    return requestedTextRectSize;
}

bool UIStaticTextComponent::IsForceBiDiSupportEnabled() const
{
    return forceBiDiSupport;
}

void UIStaticTextComponent::SetModified(bool value)
{
    modified = value;
}

bool UIStaticTextComponent::IsModified() const
{
    return modified;
}

void UIStaticTextComponent::SetState(UIStaticTextState* state_)
{
    state = state_;
    modified = true;
}

UIStaticTextState* UIStaticTextComponent::GetState() const
{
    return state;
}

// Backward compatibility method
Vector2 UIStaticTextComponent::GetContentPreferredSize(const Vector2& constraints) const
{
    DVASSERT(state);
    return state->GetTextBlock()->GetPreferredSizeForWidth(constraints.x);
}

// Backward compatibility method
bool UIStaticTextComponent::IsHeightDependsOnWidth() const
{
    DVASSERT(state);
    return state->GetTextBlock()->GetMultiline();
}
};
