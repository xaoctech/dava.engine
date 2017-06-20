#include "Utils/Utils.h"
#include "UI/UIStaticText.h"
#include "Base/ObjectFactory.h"
#include "Utils/StringFormat.h"
#include "FileSystem/LocalizationSystem.h"
#include "Render/2D/FontManager.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Animation/LinearAnimation.h"
#include "Utils/StringUtils.h"
#include "Render/2D/TextBlockSoftwareRender.h"
#include "Render/RenderHelper.h"
#include "UI/UIControlSystem.h"
#include "Job/JobManager.h"
#include "Utils/UTF8Utils.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UI/Text/UITextComponent.h"
#include "UI/Text/Private/UITextSystemLink.h"

namespace DAVA
{
const Vector2 UIStaticText::NO_REQUIRED_SIZE = Vector2(-1.f, -1.f);
const Vector2 UIStaticText::REQUIRED_CONTROL_SIZE = Vector2::Zero;
const Vector2 UIStaticText::REQUIRED_CONTROL_WIDTH = Vector2(0.f, -1.f);
const Vector2 UIStaticText::REQUIRED_CONTROL_HEIGHT = Vector2(-1.f, 0.f);

#if defined(LOCALIZATION_DEBUG)
const float32 UIStaticText::LOCALIZATION_RESERVED_PORTION = 0.6f;
const Color UIStaticText::HIGHLIGHT_COLORS[] = { DAVA::Color(1.0f, 0.0f, 0.0f, 0.4f),
                                                 DAVA::Color(0.0f, 0.0f, 1.0f, 0.4f),
                                                 DAVA::Color(1.0f, 1.0f, 0.0f, 0.4f),
                                                 DAVA::Color(1.0f, 1.0f, 1.0f, 0.4f),
                                                 DAVA::Color(1.0f, 0.0f, 1.0f, 0.4f),
                                                 DAVA::Color(0.0f, 1.0f, 0.0f, 0.4f) };
#endif

DAVA_VIRTUAL_REFLECTION_IMPL(UIStaticText)
{
    ReflectionRegistrator<UIStaticText>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIStaticText* o) { o->Release(); })
    .Field("textColor", &UIStaticText::GetTextColor, &UIStaticText::SetTextColor)
    .Field("textcolorInheritType", &UIStaticText::GetTextColorInheritType, &UIStaticText::SetTextColorInheritType)[M::EnumT<UIControlBackground::eColorInheritType>()] // TODO: camel style
    .Field("textperPixelAccuracyType", &UIStaticText::GetTextPerPixelAccuracyType, &UIStaticText::SetTextPerPixelAccuracyType)[M::EnumT<UIControlBackground::ePerPixelAccuracyType>()] // TODO: camel style
    .Field("shadowoffset", &UIStaticText::GetShadowOffset, &UIStaticText::SetShadowOffset) // TODO: camel style
    .Field("shadowcolor", &UIStaticText::GetShadowColor, &UIStaticText::SetShadowColor) // TODO: camel style
    .Field("multiline", &UIStaticText::GetMultilineType, &UIStaticText::SetMultilineType)[M::EnumT<eMultiline>()]
    .Field("fitting", &UIStaticText::GetFittingOption, &UIStaticText::SetFittingOption)[M::FlagsT<TextBlock::eFitType>()]
    .Field("textalign", &UIStaticText::GetTextAlign, &UIStaticText::SetTextAlign)[M::FlagsT<eAlign>()] // TODO: camel style
    .Field("textUseRtlAlign", &UIStaticText::GetTextUseRtlAlign, &UIStaticText::SetTextUseRtlAlign)[M::EnumT<TextBlock::eUseRtlAlign>()]
    .Field("text", &UIStaticText::GetUtf8Text, &UIStaticText::SetUtf8TextWithoutRect)
    .Field("font", &UIStaticText::GetFontPresetName, &UIStaticText::SetFontByPresetName)
    .Field("forceBiDiSupport", &UIStaticText::IsForceBiDiSupportEnabled, &UIStaticText::SetForceBiDiSupportEnabled)
    .End();
}

UIStaticText::UIStaticText(const Rect& rect)
    : UIControl(rect)
{
    text = GetOrCreateComponent<UITextComponent>();
}

UIStaticText::~UIStaticText()
{
    text = nullptr;
}

void UIStaticText::LoadFromYamlNodeCompleted()
{
    // Guard check for prevent usage text components in yaml
    DVASSERT(text == GetComponent<UITextComponent>());
}

UIStaticText* UIStaticText::Clone()
{
    UIStaticText* t = new UIStaticText(GetRect());
    t->CopyDataFrom(this);
    return t;
}

void UIStaticText::CopyDataFrom(UIControl* srcControl)
{
    UIControl::CopyDataFrom(srcControl);
    UIStaticText* src = static_cast<UIStaticText*>(srcControl);
    DVASSERT(src);

    UITextComponent* srcTextComponent = src->GetComponent<UITextComponent>();
    DVASSERT(srcTextComponent);

    RemoveComponent<UITextComponent>();
    UITextComponent* newComponent = srcTextComponent->Clone();
    AddComponent(newComponent);
    newComponent->Release();
    text = newComponent;
}

void UIStaticText::SetText(const WideString& _string, const Vector2& requestedTextRectSize /* = Vector2(0,0)*/)
{
    text->SetRequestedTextRectSize(requestedTextRectSize);
    text->SetText(UTF8Utils::EncodeToUTF8(_string));
}

void UIStaticText::SetUtf8Text(const String& utf8String, const Vector2& requestedTextRectSize /*= Vector2::Zero*/)
{
    text->SetRequestedTextRectSize(requestedTextRectSize);
    text->SetText(utf8String);
}

void UIStaticText::SetUtf8TextWithoutRect(const String& utf8String)
{
    SetUtf8Text(utf8String, Vector2::Zero);
}

String UIStaticText::GetUtf8Text() const
{
    return text->GetText();
}

void UIStaticText::SetFittingOption(int32 fittingType)
{
    switch (fittingType)
    {
    default:
        text->SetFitting(UITextComponent::eTextFitting::FITTING_NONE);
        break;
    case TextBlock::eFitType::FITTING_ENLARGE:
        text->SetFitting(UITextComponent::eTextFitting::FITTING_ENLARGE);
        break;
    case TextBlock::eFitType::FITTING_REDUCE:
        text->SetFitting(UITextComponent::eTextFitting::FITTING_REDUCE);
        break;
    case TextBlock::eFitType::FITTING_REDUCE | TextBlock::eFitType::FITTING_ENLARGE:
        text->SetFitting(UITextComponent::eTextFitting::FITTING_FILL);
        break;
    case TextBlock::eFitType::FITTING_POINTS:
        text->SetFitting(UITextComponent::eTextFitting::FITTING_POINTS);
        break;
    }
}

int32 UIStaticText::GetFittingOption() const
{
    switch (text->GetFitting())
    {
    default:
    case UITextComponent::eTextFitting::FITTING_NONE:
        return 0;
    case UITextComponent::eTextFitting::FITTING_ENLARGE:
        return TextBlock::eFitType::FITTING_ENLARGE;
    case UITextComponent::eTextFitting::FITTING_REDUCE:
        return TextBlock::eFitType::FITTING_REDUCE;
    case UITextComponent::eTextFitting::FITTING_FILL:
        return TextBlock::eFitType::FITTING_REDUCE | TextBlock::eFitType::FITTING_ENLARGE;
    case UITextComponent::eTextFitting::FITTING_POINTS:
        return TextBlock::eFitType::FITTING_POINTS;
    }
}

void UIStaticText::SetFont(Font* _font)
{
    text->SetFont(_font);
}

void UIStaticText::SetTextColor(const Color& color)
{
    text->SetColor(color);
}

void UIStaticText::SetShadowOffset(const Vector2& offset)
{
    text->SetShadowOffset(offset);
}

void UIStaticText::SetShadowColor(const Color& color)
{
    text->SetShadowColor(color);
}

void UIStaticText::SetMultiline(bool _isMultilineEnabled, bool bySymbol)
{
    if (_isMultilineEnabled && bySymbol)
    {
        text->SetMultiline(UITextComponent::eTextMultiline::MULTILINE_ENABLED_BY_SYMBOL);
    }
    else if (_isMultilineEnabled)
    {
        text->SetMultiline(UITextComponent::eTextMultiline::MULTILINE_ENABLED);
    }
    else
    {
        text->SetMultiline(UITextComponent::eTextMultiline::MULTILINE_DISABLED);
    }
}

bool UIStaticText::GetMultiline() const
{
    return text->GetMultiline() != UITextComponent::eTextMultiline::MULTILINE_DISABLED;
}

bool UIStaticText::GetMultilineBySymbol() const
{
    return text->GetMultiline() == UITextComponent::eTextMultiline::MULTILINE_ENABLED_BY_SYMBOL;
}

void UIStaticText::SetTextAlign(int32 _align)
{
    text->SetAlign(_align);
}

int32 UIStaticText::GetTextAlign() const
{
    return text->GetAlign();
}

int32 UIStaticText::GetTextVisualAlign() const
{
    return GetTextBlock()->GetVisualAlign();
}

const WideString& UIStaticText::GetVisualText() const
{
    return GetTextBlock()->GetVisualText();
}

bool UIStaticText::GetTextIsRtl() const
{
    return GetTextBlock()->IsRtl();
}

void UIStaticText::SetTextUseRtlAlign(TextBlock::eUseRtlAlign useRtlAlign)
{
    text->SetUseRtlAlign(useRtlAlign);
}

TextBlock::eUseRtlAlign UIStaticText::GetTextUseRtlAlign() const
{
    return text->GetUseRtlAlign();
}

void UIStaticText::SetTextUseRtlAlignFromInt(int32 value)
{
    SetTextUseRtlAlign(static_cast<TextBlock::eUseRtlAlign>(value));
}

int32 UIStaticText::GetTextUseRtlAlignAsInt() const
{
    return GetTextUseRtlAlign();
}

const Vector2& UIStaticText::GetTextSize()
{
    return GetTextBlock()->GetTextSize();
}

const Color& UIStaticText::GetTextColor() const
{
    return text->GetColor();
}

const Color& UIStaticText::GetShadowColor() const
{
    return text->GetShadowColor();
}

const Vector2& UIStaticText::GetShadowOffset() const
{
    return text->GetShadowOffset();
}

const Vector<WideString>& UIStaticText::GetMultilineStrings() const
{
    return GetTextBlock()->GetMultilineStrings();
}

const WideString& UIStaticText::GetText() const
{
    return GetTextBlock()->GetText();
}

// TODO fix
static Color color;
static Color colorShadow;

Animation* UIStaticText::TextColorAnimation(const Color& finalColor, float32 time, Interpolation::FuncType interpolationFunc /*= Interpolation::LINEAR*/, int32 track /*= 0*/)
{
    LinearAnimation<Color>* animation = new LinearAnimation<Color>(this, &color, finalColor, time, interpolationFunc);
    animation->Start(track);
    return animation;
}

Animation* UIStaticText::ShadowColorAnimation(const Color& finalColor, float32 time, Interpolation::FuncType interpolationFunc /*= Interpolation::LINEAR*/, int32 track /*= 1*/)
{
    LinearAnimation<Color>* animation = new LinearAnimation<Color>(this, &colorShadow, finalColor, time, interpolationFunc);
    animation->Start(track);
    return animation;
}

const Vector<float32>& UIStaticText::GetStringSizes() const
{
    return GetTextBlock()->GetStringSizes();
}

void UIStaticText::SetForceBiDiSupportEnabled(bool value)
{
    text->SetForceBiDiSupportEnabled(value);
}

void UIStaticText::SetMeasureEnable(bool value)
{
    text->GetLink()->GetTextBlock()->SetMeasureEnable(value);
}

String UIStaticText::GetFontPresetName() const
{
    return text->GetFontName();
}

void UIStaticText::SetFontByPresetName(const String& presetName)
{
    text->SetFontName(presetName);
}

int32 UIStaticText::GetTextColorInheritType() const
{
    return text->GetColorInheritType();
}

void UIStaticText::SetTextColorInheritType(int32 type)
{
    text->SetColorInheritType(static_cast<UIControlBackground::eColorInheritType>(type));
}

int32 UIStaticText::GetTextPerPixelAccuracyType() const
{
    return text->GetPerPixelAccuracyType();
}

void UIStaticText::SetTextPerPixelAccuracyType(int32 type)
{
    text->SetPerPixelAccuracyType(static_cast<UIControlBackground::ePerPixelAccuracyType>(type));
}

int32 UIStaticText::GetMultilineType() const
{
    switch (text->GetMultiline())
    {
    default:
    case UITextComponent::eTextMultiline::MULTILINE_DISABLED:
        return eMultiline::MULTILINE_DISABLED;
    case UITextComponent::eTextMultiline::MULTILINE_ENABLED:
        return eMultiline::MULTILINE_ENABLED;
    case UITextComponent::eTextMultiline::MULTILINE_ENABLED_BY_SYMBOL:
        return eMultiline::MULTILINE_ENABLED_BY_SYMBOL;
    }
}

void UIStaticText::SetMultilineType(int32 multilineType)
{
    switch (multilineType)
    {
    default:
    case eMultiline::MULTILINE_DISABLED:
        text->SetMultiline(UITextComponent::eTextMultiline::MULTILINE_DISABLED);
        break;
    case eMultiline::MULTILINE_ENABLED:
        text->SetMultiline(UITextComponent::eTextMultiline::MULTILINE_ENABLED);
        break;
    case eMultiline::MULTILINE_ENABLED_BY_SYMBOL:
        text->SetMultiline(UITextComponent::eTextMultiline::MULTILINE_ENABLED_BY_SYMBOL);
        break;
    }
}

bool UIStaticText::IsForceBiDiSupportEnabled() const
{
    return text->IsForceBiDiSupportEnabled();
}
DAVA::Font* UIStaticText::GetFont() const
{
    return GetTextBlock()->GetFont();
}

DAVA::float32 UIStaticText::GetFontSize() const
{
    return GetTextBlock()->GetRenderSize();
}
void UIStaticText::SetFontSize(float32 newSize)
{
    GetTextBlock()->SetRenderSize(newSize);
}

TextBox* UIStaticText::GetTextBox()
{
    return GetTextBlock()->GetTextBox();
}

TextBlock* UIStaticText::GetTextBlock() const
{
    text->ApplyDataImmediately();
    return text->GetLink()->GetTextBlock();
}
}
