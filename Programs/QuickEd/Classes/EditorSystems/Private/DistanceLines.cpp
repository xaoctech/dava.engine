#include "EditorSystems/Private/DistanceLines.h"
#include "EditorSystems/DistanceSystem.h"
#include "EditorSystems/UIControlUtils.h"

#include <TArc/Core/ContextAccessor.h>

#include <UI/UIStaticText.h>
#include <Utils/UTF8Utils.h>

SolidLine::SolidLineParams::SolidLineParams(const DAVA::UIGeometricData& gd_)
    : gd(gd_)
{
}

SolidLine::SolidLine(const SolidLineParams& params)
    : accessor(params.accessor)
    , startPoint(params.startPoint)
    , endPoint(params.endPoint)
    , font(params.font)
    , parentGd(params.gd)
    , direction(params.direction)
{
}

void SolidLine::Draw(DAVA::UIControl* canvas)
{
    DrawSolidLine(canvas);
    DrawEndLine(canvas);
    DrawLineText(canvas);
}

void SolidLine::DrawSolidLine(DAVA::UIControl* canvas)
{
    using namespace DAVA;

    DistanceSystemPreferences* preferences = accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    RefPtr<UIControl> lineControl = UIControlUtils::CreateLineWithColor(preferences->linesColor, "distance_line");

    Vector2 linePos(std::min(startPoint.x, endPoint.x), std::min(startPoint.y, endPoint.y));
    Vector2 lineSize(fabs(startPoint.x - endPoint.x), fabs(startPoint.y - endPoint.y));
    UIControlUtils::MapToScreen(Rect(linePos, lineSize), parentGd, lineControl);

    canvas->AddControl(lineControl.Get());
}

void SolidLine::DrawEndLine(DAVA::UIControl* canvas)
{
    using namespace DAVA;

    const Vector2 endLineLength = Vector2(8.0f, 8.0f) / parentGd.scale;
    DistanceSystemPreferences* preferences = accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    RefPtr<UIControl> lineControl = UIControlUtils::CreateLineWithColor(preferences->linesColor, "distance_line_ending");

    if (direction == ALIGN_BOTTOM || direction == ALIGN_TOP)
    {
        Vector2 linePos(endPoint.x - endLineLength.dx / 2.0f, endPoint.y);
        Vector2 lineSize(endLineLength.dy, 0.0f);
        UIControlUtils::MapToScreen(Rect(linePos, lineSize), parentGd, lineControl);
    }
    else if (direction == ALIGN_LEFT || direction == ALIGN_RIGHT)
    {
        Vector2 linePos(endPoint.x, endPoint.y - endLineLength.dx / 2.0f);
        Vector2 lineSize(0.0f, endLineLength.dy);
        UIControlUtils::MapToScreen(Rect(linePos, lineSize), parentGd, lineControl);
    }
    else
    {
        DVASSERT("wrong direction value");
    }

    canvas->AddControl(lineControl.Get());
}

void SolidLine::DrawLineText(DAVA::UIControl* canvas)
{
    using namespace DAVA;

    float32 length = (endPoint - startPoint).Length();

    RefPtr<UIStaticText> textControl(new UIStaticText());
    textControl->SetTextColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
    textControl->SetTextPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_FORCED);
    DistanceSystemPreferences* preferences = accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    textControl->SetTextColor(preferences->textColor);
    textControl->SetFont(font.Get());

    String text = Format("%.0f", length);

    textControl->SetUtf8Text(text);

    Font::StringMetrics metrics = font->GetStringMetrics(UTF8Utils::EncodeToWideString(text));
    Vector2 size(metrics.width, metrics.height);

    float32 maxLength = 13;
    float32 offset = 2;
    Vector2::eAxis axis = direction == ALIGN_TOP || direction == ALIGN_BOTTOM ? Vector2::AXIS_X : Vector2::AXIS_Y;
    Vector2::eAxis oppositeAxis = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;
    Vector2 pos;
    if (length > maxLength)
    {
        pos[axis] = direction == ALIGN_TOP || direction == ALIGN_RIGHT ?
        startPoint[axis] + offset :
        startPoint[axis] - size[axis] - offset;
        pos[oppositeAxis] = (startPoint[oppositeAxis] + endPoint[oppositeAxis]) / 2.0f - size[oppositeAxis] / 2.0f;
    }
    else
    {
        pos[axis] = startPoint[axis] - (size[axis] / 2.0f);
        pos[oppositeAxis] = direction == ALIGN_BOTTOM || direction == ALIGN_RIGHT ?
        endPoint[oppositeAxis] + offset :
        endPoint[oppositeAxis] - size[oppositeAxis] - offset;
    }
    UIControlUtils::MapToScreen(Rect(pos, size), parentGd, textControl);
    canvas->AddControl(textControl.Get());
}

DotLine::DotLineParams::DotLineParams(const DAVA::UIGeometricData& gd_)
    : gd(gd_)
{
}

DotLine::DotLine(const DotLineParams& params)
    : accessor(params.accessor)
    , startPoint(params.startPoint)
    , endPoint(params.endPoint)
    , parentGd(params.gd)
{
}

void DotLine::Draw(DAVA::UIControl* canvas)
{
    using namespace DAVA;

    DistanceSystemPreferences* preferences = accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    RefPtr<UIControl> lineControl = UIControlUtils::CreateLineWithTexture(preferences->helpLinesTexture, "dot_distance_line");

    Vector2 linePos(std::min(startPoint.x, endPoint.x), std::min(startPoint.y, endPoint.y));
    Vector2 lineSize(fabs(startPoint.x - endPoint.x), fabs(startPoint.y - endPoint.y));
    UIControlUtils::MapToScreen(Rect(linePos, lineSize), parentGd, lineControl);

    canvas->AddControl(lineControl.Get());
}
