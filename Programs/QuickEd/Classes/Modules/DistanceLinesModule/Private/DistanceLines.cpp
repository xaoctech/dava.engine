#include "EditorSystems/Private/DistanceLines.h"
#include "EditorSystems/DistanceSystem.h"
#include "EditorSystems/UIControlUtils.h"

#include <TArc/Core/ContextAccessor.h>

#include <UI/UIStaticText.h>
#include <Utils/UTF8Utils.h>

namespace DistanceLinesDetails
{
//control highlight is drawed inside of control
//in some params.directions line can be under highlight
void FixLinePosition(DAVA::Vector2& pos, const DAVA::UIGeometricData& gd, DAVA::eAlign direction)
{
    using namespace DAVA;
    if (direction == ALIGN_BOTTOM)
    {
        pos.y -= 1.0f / gd.scale.y;
    }
    else if (direction == ALIGN_RIGHT)
    {
        pos.x -= 1.0f / gd.scale.x;
    }
}
}

LineParams::LineParams(const DAVA::UIGeometricData& gd_)
    : gd(gd_)
{
}

DistanceLine::DistanceLine(const LineParams& params)
    : params(params)
{
}

SolidLine::SolidLine(const LineParams& params_)
    : DistanceLine(params_)
{
    DistanceLinesDetails::FixLinePosition(params.endPoint, params.gd, params.direction);
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

    DistanceSystemPreferences* preferences = params.accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    RefPtr<UIControl> lineControl = UIControlUtils::CreateLineWithColor(preferences->linesColor, "distance_line");

    Vector2 linePos(std::min(params.startPoint.x, params.endPoint.x), std::min(params.startPoint.y, params.endPoint.y));
    Vector2 lineSize(fabs(params.startPoint.x - params.endPoint.x), fabs(params.startPoint.y - params.endPoint.y));
    UIControlUtils::MapLineToScreen(params.axis, Rect(linePos, lineSize), params.gd, lineControl);

    canvas->AddControl(lineControl.Get());
}

void SolidLine::DrawEndLine(DAVA::UIControl* canvas)
{
    using namespace DAVA;

    const Vector2 endLineLength = Vector2(8.0f, 8.0f);
    DistanceSystemPreferences* preferences = params.accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    RefPtr<UIControl> lineControl = UIControlUtils::CreateLineWithColor(preferences->linesColor, "distance_line_ending");

    Vector2 linePos = params.endPoint;
    Vector2 lineSize;
    lineSize[params.oppositeAxis] = endLineLength[params.oppositeAxis] / params.gd.scale[params.oppositeAxis];
    lineSize[params.axis] = 0.0f;
    linePos[params.oppositeAxis] -= lineSize[params.oppositeAxis] / 2.0f;
    UIControlUtils::MapLineToScreen(params.oppositeAxis, Rect(linePos, lineSize), params.gd, lineControl);

    canvas->AddControl(lineControl.Get());
}

void SolidLine::DrawLineText(DAVA::UIControl* canvas)
{
    using namespace DAVA;

    float32 length = fabs((params.endPoint - params.startPoint)[params.axis]);

    RefPtr<UIStaticText> textControl(new UIStaticText());
    textControl->SetTextColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
    textControl->SetTextPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_FORCED);
    DistanceSystemPreferences* preferences = params.accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    textControl->SetTextColor(preferences->textColor);
    textControl->SetFont(params.font.Get());

    String text = Format("%.0f", length);

    textControl->SetUtf8Text(text);

    Font::StringMetrics metrics = params.font->GetStringMetrics(UTF8Utils::EncodeToWideString(text));
    Vector2 size(metrics.width, metrics.height);
    size /= params.gd.scale;

    //margin around text
    Vector2 margin = Vector2(3.0f, 3.0f) / params.gd.scale;
    Vector2 pos;

    if (length > (size[params.axis] + margin[params.axis]))
    {
        pos[params.oppositeAxis] = params.direction == ALIGN_TOP || params.direction == ALIGN_RIGHT ?
        params.startPoint[params.oppositeAxis] + margin[params.oppositeAxis] :
        params.startPoint[params.oppositeAxis] - size[params.oppositeAxis] - margin[params.oppositeAxis];
        pos[params.axis] = (params.startPoint[params.axis] + params.endPoint[params.axis]) / 2.0f - size[params.axis] / 2.0f;
    }
    else
    {
        pos[params.oppositeAxis] = params.startPoint[params.oppositeAxis] - (size[params.oppositeAxis] / 2.0f);
        pos[params.axis] = params.direction == ALIGN_BOTTOM || params.direction == ALIGN_RIGHT ?
        params.endPoint[params.axis] + margin[params.axis] :
        params.endPoint[params.axis] - size[params.axis] - margin[params.axis];
    }
    UIControlUtils::MapRectToScreen(Rect(pos, size), params.gd, textControl);
    canvas->AddControl(textControl.Get());
}

DotLine::DotLine(const LineParams& params)
    : DistanceLine(params)
{
}

void DotLine::Draw(DAVA::UIControl* canvas)
{
    using namespace DAVA;

    DistanceSystemPreferences* preferences = params.accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    RefPtr<UIControl> lineControl = UIControlUtils::CreateLineWithTexture(preferences->helpLinesTexture, "dot_distance_line");

    Vector2 linePos(std::min(params.startPoint.x, params.endPoint.x), std::min(params.startPoint.y, params.endPoint.y));
    Vector2 lineSize(fabs(params.startPoint.x - params.endPoint.x), fabs(params.startPoint.y - params.endPoint.y));
    UIControlUtils::MapLineToScreen(params.axis, Rect(linePos, lineSize), params.gd, lineControl);

    canvas->AddControl(lineControl.Get());
}
