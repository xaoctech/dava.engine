#include "Classes/Modules/DistanceLinesModule/Private/DistanceLines.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceSystem.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceLinesPreferences.h"

#include "Classes/EditorSystems/UIControlUtils.h"

#include "Classes/Painter/Painter.h"

#include <TArc/Core/ContextAccessor.h>

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

DistanceLine::~DistanceLine()
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

    Painting::DrawTextParams textParams;

    DistanceSystemPreferences* preferences = params.accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    textParams.color = preferences->textColor;
    textParams.text = Format("%.0f", length);
    textParams.margin = Vector2(3.0f, 3.0f);
    textParams.angle = params.gd.angle;
    textParams.scale = params.gd.scale;
    textParams.parentPos = params.gd.position - Rotate(params.gd.pivotPoint * params.gd.scale, params.gd.angle);

    //margin around text
    const float32 minLength = 20.0f;
    if (length > minLength)
    {
        if (params.axis == Vector2::AXIS_X)
        {
            textParams.direction = ALIGN_HCENTER | (params.direction == ALIGN_RIGHT ? ALIGN_BOTTOM : ALIGN_TOP);
        }
        else
        {
            textParams.direction = ALIGN_VCENTER | (params.direction == ALIGN_BOTTOM ? ALIGN_LEFT : ALIGN_RIGHT);
        }

        textParams.pos[params.axis] = (params.startPoint[params.axis] + params.endPoint[params.axis]) / 2.0f;
        textParams.pos[params.oppositeAxis] = params.endPoint[params.oppositeAxis];
    }
    else
    {
        textParams.direction = params.direction | (params.axis == Vector2::AXIS_X ? ALIGN_VCENTER : ALIGN_HCENTER);
        textParams.pos[params.axis] = params.endPoint[params.axis];
        textParams.pos[params.oppositeAxis] = params.endPoint[params.oppositeAxis];
    }
    params.painter->Add(textParams);
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
