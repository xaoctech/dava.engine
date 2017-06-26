#include "UI/Text/UITextComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UITextSystemLink.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UITextComponent)
{
    ReflectionRegistrator<UITextComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UITextComponent* o) { o->Release(); })
    .Field("align", &UITextComponent::GetAlign, &UITextComponent::SetAlign)[M::FlagsT<eAlign>()]
    .Field("color", &UITextComponent::GetColor, &UITextComponent::SetColor)
    .Field("colorInheritType", &UITextComponent::GetColorInheritType, &UITextComponent::SetColorInheritType)[M::EnumT<UIControlBackground::eColorInheritType>()]
    .Field("fitting", &UITextComponent::GetFitting, &UITextComponent::SetFitting)[M::EnumT<eTextFitting>()]
    .Field("fontName", &UITextComponent::GetFontName, &UITextComponent::SetFontName)
    .Field("forceBiDiSupport", &UITextComponent::IsForceBiDiSupportEnabled, &UITextComponent::SetForceBiDiSupportEnabled)
    .Field("multiline", &UITextComponent::GetMultiline, &UITextComponent::SetMultiline)[M::EnumT<eTextMultiline>()]
    .Field("perPixelAccuracyType", &UITextComponent::GetPerPixelAccuracyType, &UITextComponent::SetPerPixelAccuracyType)[M::EnumT<UIControlBackground::ePerPixelAccuracyType>()]
    .Field("shadowColor", &UITextComponent::GetShadowColor, &UITextComponent::SetShadowColor)
    .Field("shadowOffset", &UITextComponent::GetShadowOffset, &UITextComponent::SetShadowOffset)
    .Field("text", &UITextComponent::GetText, &UITextComponent::SetText)
    .Field("useRtlAlign", &UITextComponent::GetUseRtlAlign, &UITextComponent::SetUseRtlAlign)[M::EnumT<TextBlock::eUseRtlAlign>()]
    .End();
}

UITextComponent::UITextComponent(const UITextComponent& src)
    : UIComponent(src)
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

UITextComponent* UITextComponent::Clone() const
{
    return new UITextComponent(*this);
}

void UITextComponent::SetAlign(int32 value)
{
    if (align != value)
    {
        align = value;
        modified = true;
    }
}

int32 UITextComponent::GetAlign() const
{
    return align;
}

void UITextComponent::SetText(const String& value)
{
    // force set modified flag
    text = value;
    modified = true;
}

String UITextComponent::GetText() const
{
    return text;
}

void UITextComponent::SetFitting(eTextFitting value)
{
    if (fitting != value)
    {
        fitting = value;
        modified = true;
    }
}

UITextComponent::eTextFitting UITextComponent::GetFitting() const
{
    return fitting;
}

void UITextComponent::SetFontName(const String& value)
{
    if (fontName != value)
    {
        fontName = value;
        modified = true;
    }
}

String UITextComponent::GetFontName() const
{
    return fontName;
}

void UITextComponent::SetColor(const Color& value)
{
    if (color != value)
    {
        color = value;
        modified = true;
    }
}

const Color& UITextComponent::GetColor() const
{
    return color;
}

void UITextComponent::SetMultiline(eTextMultiline value)
{
    if (multiline != value)
    {
        multiline = value;
        modified = true;
    }
}

UITextComponent::eTextMultiline UITextComponent::GetMultiline() const
{
    return multiline;
}

void UITextComponent::SetColorInheritType(UIControlBackground::eColorInheritType value)
{
    if (colorInheritType != value)
    {
        colorInheritType = value;
        modified = true;
    }
}

UIControlBackground::eColorInheritType UITextComponent::GetColorInheritType() const
{
    return colorInheritType;
}

void UITextComponent::SetShadowOffset(const Vector2& value)
{
    if (shadowOffset != value)
    {
        shadowOffset = value;
        modified = true;
    }
}

const Vector2& UITextComponent::GetShadowOffset() const
{
    return shadowOffset;
}

void UITextComponent::SetShadowColor(const Color& value)
{
    if (shadowColor != value)
    {
        shadowColor = value;
        modified = true;
    }
}

const Color& UITextComponent::GetShadowColor() const
{
    return shadowColor;
}

void UITextComponent::SetPerPixelAccuracyType(UIControlBackground::ePerPixelAccuracyType value)
{
    if (perPixelAccuracyType != value)
    {
        perPixelAccuracyType = value;
        modified = true;
    }
}

UIControlBackground::ePerPixelAccuracyType UITextComponent::GetPerPixelAccuracyType() const
{
    return perPixelAccuracyType;
}

void UITextComponent::SetUseRtlAlign(TextBlock::eUseRtlAlign value)
{
    if (useRtlAlign != value)
    {
        useRtlAlign = value;
        modified = true;
    }
}

TextBlock::eUseRtlAlign UITextComponent::GetUseRtlAlign() const
{
    return useRtlAlign;
}

void UITextComponent::SetForceBiDiSupportEnabled(bool value)
{
    if (forceBiDiSupport != value)
    {
        forceBiDiSupport = value;
        modified = true;
    }
}

void UITextComponent::SetRequestedTextRectSize(const Vector2& value)
{
    if (requestedTextRectSize != value)
    {
        requestedTextRectSize = value;
        modified = true;
    }
}

DAVA::Vector2 UITextComponent::GetRequestedTextRectSize() const
{
    return requestedTextRectSize;
}

bool UITextComponent::IsForceBiDiSupportEnabled() const
{
    return forceBiDiSupport;
}

void UITextComponent::SetModified(bool value)
{
    modified = value;
}

bool UITextComponent::IsModified() const
{
    return modified;
}

void UITextComponent::SetLink(UITextSystemLink* value)
{
    link = value;
    modified = true;
}

UITextSystemLink* UITextComponent::GetLink() const
{
    return link;
}

// Backward compatibility method
Vector2 UITextComponent::GetContentPreferredSize(const Vector2& constraints) const
{
    DVASSERT(link);
    return link->GetTextBlock()->GetPreferredSizeForWidth(constraints.x);
}

// Backward compatibility method
bool UITextComponent::IsHeightDependsOnWidth() const
{
    DVASSERT(link);
    return link->GetTextBlock()->GetMultiline();
}
};
