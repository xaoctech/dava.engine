#include "UIStaticTextState.h"
#include "UIStaticTextComponent.h"

#include "UI/UIControl.h"
#include "UI/Layouts/LayoutFormula.h"
#include "Math/Vector.h"

#include "Utils/Utils.h"
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
#include "Math\Math2D.h"

namespace DAVA
{

#if defined(LOCALIZATION_DEBUG)
const float32 UIStaticTextState::LOCALIZATION_RESERVED_PORTION = 0.6f;
const Color UIStaticTextState::HIGHLIGHT_COLORS[] = { DAVA::Color(1.0f, 0.0f, 0.0f, 0.4f),
                                                      DAVA::Color(0.0f, 0.0f, 1.0f, 0.4f),
                                                      DAVA::Color(1.0f, 1.0f, 0.0f, 0.4f),
                                                      DAVA::Color(1.0f, 1.0f, 1.0f, 0.4f),
                                                      DAVA::Color(1.0f, 0.0f, 1.0f, 0.4f),
                                                      DAVA::Color(0.0f, 1.0f, 0.0f, 0.4f) };
#endif

UIStaticTextState::UIStaticTextState(UIControl* control_, UIStaticTextComponent* component_)
{
    control = control_;
    component = component_;
    Rect rect = control->GetRect();
    control->SetInputEnabled(false, false);
    textBlock = TextBlock::Create(Vector2(rect.dx, rect.dy));

    textBg = new UIControlBackground();
    textBg->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    textBg->SetColorInheritType(component->GetColorInheritType());
    textBg->SetPerPixelAccuracyType(component->GetPerPixelAccuracyType());

    shadowBg = new UIControlBackground();
    shadowBg->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    shadowBg->SetColorInheritType(component->GetColorInheritType());
    shadowBg->SetPerPixelAccuracyType(component->GetPerPixelAccuracyType());
}

UIStaticTextState::~UIStaticTextState()
{
    SafeRelease(textBlock);
    SafeRelease(shadowBg);
    SafeRelease(textBg);
}

void UIStaticTextState::ApplyComponentData()
{
    if (component->IsModified())
    {
        component->SetModified(false);

        textBg->SetColorInheritType(component->GetColorInheritType());
        textBg->SetPerPixelAccuracyType(component->GetPerPixelAccuracyType());
        textBg->SetColor(component->GetColor());

        shadowBg->SetColorInheritType(component->GetColorInheritType());
        shadowBg->SetPerPixelAccuracyType(component->GetPerPixelAccuracyType());
        shadowBg->SetColor(component->GetShadowColor());

        textBlock->SetRectSize(control->size);

        switch (component->GetFitting())
        {
        default:
        case UIStaticTextComponent::eTextFitting::FITTING_NONE:
            textBlock->SetFittingOption(0);
            break;
        case UIStaticTextComponent::eTextFitting::FITTING_ENLARGE:
            textBlock->SetFittingOption(TextBlock::eFitType::FITTING_ENLARGE);
            break;
        case UIStaticTextComponent::eTextFitting::FITTING_REDUCE:
            textBlock->SetFittingOption(TextBlock::eFitType::FITTING_REDUCE);
            break;
        case UIStaticTextComponent::eTextFitting::FITTING_FILL:
            textBlock->SetFittingOption(TextBlock::eFitType::FITTING_REDUCE | TextBlock::eFitType::FITTING_ENLARGE);
            break;
        case UIStaticTextComponent::eTextFitting::FITTING_POINTS:
            textBlock->SetFittingOption(TextBlock::eFitType::FITTING_POINTS);
            break;
        }

        textBlock->SetText(UTF8Utils::EncodeToWideString(component->GetText()), component->GetRequestedTextRectSize());

        String fontName = component->GetFontName();
        if (!fontName.empty())
        {
            Font* font = FontManager::Instance()->GetFont(fontName);
            if (textBlock->GetFont() != font)
            {
                textBlock->SetFont(font);
            }
        }

        switch (component->GetMultiline())
        {
        default:
        case UIStaticTextComponent::eTextMultiline::MULTILINE_DISABLED:
            textBlock->SetMultiline(false, false);
            break;
        case UIStaticTextComponent::eTextMultiline::MULTILINE_ENABLED:
            textBlock->SetMultiline(true, false);
            break;
        case UIStaticTextComponent::eTextMultiline::MULTILINE_ENABLED_BY_SYMBOL:
            textBlock->SetMultiline(true, true);
            break;
        }

        textBlock->SetAlign(component->GetAlign());
        textBlock->SetUseRtlAlign(component->GetUseRtlAlign());
        textBlock->SetForceBiDiSupportEnabled(component->IsForceBiDiSupportEnabled());

        if (textBlock->NeedCalculateCacheParams())
        {
            control->SetLayoutDirty();
        }

    }
}

void UIStaticTextState::PrepareSprite()
{
    if (textBlock->IsSpriteReady())
    {
        Sprite* sprite = textBlock->GetSprite();
        shadowBg->SetSprite(sprite, 0);
        textBg->SetSprite(sprite, 0);

        Texture* tex = sprite->GetTexture();
        if (tex && tex->GetFormat() == FORMAT_A8)
        {
            textBg->SetMaterial(RenderSystem2D::DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL);
            shadowBg->SetMaterial(RenderSystem2D::DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL);
        }
        else
        {
            textBg->SetMaterial(RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL);
            shadowBg->SetMaterial(RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL);
        }
    }
    else
    {
        shadowBg->SetSprite(NULL, 0);
        textBg->SetSprite(NULL, 0);
    }
}

void UIStaticTextState::Draw(const UIGeometricData& geometricData, const Color& parentColor)
{
    DVASSERT(control == component->GetControl(), "Invalid control poiner!");

    ApplyComponentData();

    shadowBg->SetParentColor(parentColor);
    textBg->SetParentColor(parentColor);

    if (component->GetText().empty())
    {
        return;
    }

    Rect textBlockRect(geometricData.position, geometricData.size);
    if (textBlock->GetFont() && textBlock->GetFont()->GetFontType() == Font::TYPE_DISTANCE)
    {
        // Correct rect and setup position and scale for distance fonts
        textBlockRect.dx *= geometricData.scale.dx;
        textBlockRect.dy *= geometricData.scale.dy;
        textBlock->SetScale(geometricData.scale);
        textBlock->SetAngle(geometricData.angle);
        textBlock->SetPivot(control->GetPivotPoint() * geometricData.scale);
    }
    textBlock->SetRectSize(textBlockRect.GetSize());
    textBlock->SetPosition(textBlockRect.GetPosition());
    textBlock->PreDraw();
    PrepareSprite();
    textBg->SetAlign(textBlock->GetVisualAlign());

    UIGeometricData textGeomData;
    textGeomData.position = textBlock->GetSpriteOffset();
    textGeomData.size = control->GetSize();
    textGeomData.AddGeometricData(geometricData);

    Vector2 shadowOffset = component->GetShadowOffset();

    if (!FLOAT_EQUAL(shadowBg->GetDrawColor().a, 0.0f) && (!FLOAT_EQUAL(shadowOffset.dx, 0.0f) || !FLOAT_EQUAL(shadowOffset.dy, 0.0f)))
    {
        textBlock->Draw(shadowBg->GetDrawColor(), &shadowOffset);
        UIGeometricData shadowGeomData;
        shadowGeomData.position = shadowOffset;
        shadowGeomData.size = control->GetSize();
        shadowGeomData.AddGeometricData(textGeomData);

        shadowBg->SetAlign(textBg->GetAlign());
        shadowBg->Draw(shadowGeomData);
    }

    textBlock->Draw(textBg->GetDrawColor());

    textBg->Draw(textGeomData);
    
#if defined(LOCALIZATION_DEBUG)

    if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DRAW_LINEBREAK_ERRORS) || Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DRAW_LOCALIZATION_WARINGS))
    {
        DrawLocalizationDebug(geometricData);
    }
    if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DRAW_LOCALIZATION_ERRORS))
    {
        DrawLocalizationErrors(geometricData);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(LOCALIZATION_DEBUG)
void UIStaticTextState::DrawLocalizationErrors(const UIGeometricData& geometricData) const
{
    UIGeometricData elementGeomData;
    const Sprite::DrawState& lastDrawStae = textBg->GetLastDrawState();
    elementGeomData.position = lastDrawStae.position;
    elementGeomData.angle = lastDrawStae.angle;
    elementGeomData.scale = lastDrawStae.scale;
    elementGeomData.pivotPoint = lastDrawStae.pivotPoint;

    TextBlockSoftwareRender* rendereTextBlock = dynamic_cast<TextBlockSoftwareRender*>(textBlock->GetRenderer());
    if (rendereTextBlock != NULL)
    {
        DAVA::Matrix3 transform;
        elementGeomData.BuildTransformMatrix(transform);

        UIGeometricData textGeomData(elementGeomData);

        Vector3 x3 = Vector3(1.0f, 0.0f, 0.0f) * transform, y3 = Vector3(0.0f, 1.0f, 0.0f) * transform;
        Vector2 x(x3.x, x3.y), y(y3.x, y3.y);

        //reduce size by 1 pixel from each size for polygon to fit into control hence +1.0f and -1.0f
        //getTextOffsetTL and getTextOffsetBR are in physical coordinates but draw is still in virtual
        textGeomData.position += (x * UIControlSystem::Instance()->vcs->ConvertPhysicalToVirtualX(rendereTextBlock->getTextOffsetTL().x + 1.0f));
        textGeomData.position += (y * UIControlSystem::Instance()->vcs->ConvertPhysicalToVirtualY(rendereTextBlock->getTextOffsetTL().y + 1.0f));

        textGeomData.size = Vector2(0.0f, 0.0f);
        textGeomData.size.x += UIControlSystem::Instance()->vcs->ConvertPhysicalToVirtualX((rendereTextBlock->getTextOffsetBR().x - rendereTextBlock->getTextOffsetTL().x) - 1.0f);
        textGeomData.size.y += UIControlSystem::Instance()->vcs->ConvertPhysicalToVirtualY((rendereTextBlock->getTextOffsetBR().y - rendereTextBlock->getTextOffsetTL().y) - 1.0f);

        DAVA::Polygon2 textPolygon;
        textGeomData.GetPolygon(textPolygon);

        DAVA::Polygon2 controllPolygon;
        geometricData.GetPolygon(controllPolygon);

        //polygons will have te same transformation so just compare them
        if (!controllPolygon.IsPointInside(textPolygon.GetPoints()[0]) ||
            !controllPolygon.IsPointInside(textPolygon.GetPoints()[1]) ||
            !controllPolygon.IsPointInside(textPolygon.GetPoints()[2]) ||
            !controllPolygon.IsPointInside(textPolygon.GetPoints()[3]))
        {
            RenderSystem2D::Instance()->DrawPolygon(textPolygon, true, HIGHLIGHT_COLORS[MAGENTA]);
            RenderSystem2D::Instance()->FillPolygon(controllPolygon, HIGHLIGHT_COLORS[RED]);
        }
        if (textBlock->IsVisualTextCroped())
        {
            RenderSystem2D::Instance()->FillPolygon(textPolygon, HIGHLIGHT_COLORS[YELLOW]);
        }
    }
}
void UIStaticTextState::DrawLocalizationDebug(const UIGeometricData& textGeomData) const
{
    // void UIStaticTextState::RecalculateDebugColoring()
    // {
    DebugHighliteColor warningColor = NONE;
    DebugHighliteColor lineBreakError = NONE;
    if (textBlock->GetFont() == NULL)
        return;

    if (textBlock->GetMultiline())
    {
        const Vector<WideString>& strings = textBlock->GetMultilineStrings();
        const WideString& text = textBlock->GetText();
        float32 accumulatedHeight = 0.0f;
        float32 maxWidth = 0.0f;

        if (!text.empty())
        {
            WideString textNoSpaces = StringUtils::RemoveNonPrintable(text, 1);
            // StringUtils::IsWhitespace function has 2 overloads and compiler cannot deduce predicate parameter for std::remove_if
            // So help compiler to choose correct overload of StringUtils::IsWhitespace function using static_cast
            auto res = remove_if(textNoSpaces.begin(), textNoSpaces.end(), static_cast<bool (*)(WideString::value_type)>(&StringUtils::IsWhitespace));
            textNoSpaces.erase(res, textNoSpaces.end());

            WideString concatinatedStringsNoSpaces = L"";
            for (Vector<WideString>::const_iterator string = strings.begin();
                 string != strings.end(); string++)
            {
                WideString toFilter = *string;
                toFilter.erase(remove_if(toFilter.begin(), toFilter.end(), static_cast<bool (*)(WideString::value_type)>(&StringUtils::IsWhitespace)), toFilter.end());
                concatinatedStringsNoSpaces += toFilter;
            }

            if (concatinatedStringsNoSpaces != textNoSpaces)
            {
                lineBreakError = RED;
            }
        }
    }
    // }

    if (warningColor != NONE && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DRAW_LOCALIZATION_WARINGS))
    {
        DAVA::Polygon2 polygon;
        textGeomData.GetPolygon(polygon);
        RenderSystem2D::Instance()->DrawPolygon(polygon, true, HIGHLIGHT_COLORS[warningColor]);
    }
    if (lineBreakError != NONE && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DRAW_LINEBREAK_ERRORS))
    {
        DAVA::Polygon2 polygon;
        textGeomData.GetPolygon(polygon);
        RenderSystem2D::Instance()->FillPolygon(polygon, HIGHLIGHT_COLORS[lineBreakError]);
    }
    if (textBlock->GetFittingOption() != 0 && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DRAW_LOCALIZATION_WARINGS))
    {
        Color color = HIGHLIGHT_COLORS[WHITE];
        if (textBlock->GetFittingOptionUsed() != 0)
        {
            if (textBlock->GetFittingOptionUsed() & TextBlock::FITTING_REDUCE)
                color = HIGHLIGHT_COLORS[RED];
            if (textBlock->GetFittingOptionUsed() & TextBlock::FITTING_ENLARGE)
                color = HIGHLIGHT_COLORS[YELLOW];
            if (textBlock->GetFittingOptionUsed() & TextBlock::FITTING_POINTS)
                color = HIGHLIGHT_COLORS[BLUE];
        }
        DAVA::Polygon2 polygon;
        textGeomData.GetPolygon(polygon);
        DVASSERT(polygon.GetPointCount() == 4);
        RenderSystem2D::Instance()->DrawLine(polygon.GetPoints()[0], polygon.GetPoints()[2], color);
    }
}

#endif
};
