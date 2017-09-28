#include "Classes/Modules/DistanceLinesModule/Private/DistanceLines.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceSystem.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceLinesPreferences.h"

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

void SolidLine::Draw()
{
    DrawSolidLine();
    DrawEndLine();
    DrawLineText();
}

void SolidLine::DrawSolidLine()
{
    using namespace DAVA;

    Painting::DrawLineParams lineParams;

    DistanceSystemPreferences* preferences = params.accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    lineParams.color = preferences->linesColor;
    lineParams.startPos = params.startPoint;
    lineParams.endPos = params.endPoint;
    params.gd.BuildTransformMatrix(lineParams.transformMatrix);

    params.painter->Add(params.order, lineParams);
}

void SolidLine::DrawEndLine()
{
    using namespace DAVA;

    const float32 endLineLength = 8.0f;

    Painting::DrawLineParams lineParams;
    DistanceSystemPreferences* preferences = params.accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    lineParams.color = preferences->linesColor;
    lineParams.startPos = params.endPoint;
    lineParams.endPos = params.endPoint;
    lineParams.startPos[params.oppositeAxis] -= endLineLength / 2.0f / params.gd.scale[params.oppositeAxis];
    lineParams.endPos[params.oppositeAxis] += endLineLength / 2.0f / params.gd.scale[params.oppositeAxis];
    params.gd.BuildTransformMatrix(lineParams.transformMatrix);
    params.painter->Add(params.order, lineParams);
}

void SolidLine::DrawLineText()
{
    using namespace DAVA;

    Painting::DrawTextParams textParams;

    DistanceSystemPreferences* preferences = params.accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    textParams.color = preferences->textColor;
    textParams.text = Format("%.0f", params.length);
    textParams.margin = Vector2(5.0f, 5.0f);
    textParams.scale = params.gd.scale;
    textParams.angle = params.gd.angle;
    params.gd.BuildTransformMatrix(textParams.transformMatrix);

    //margin around text
    const float32 minLength = 20.0f;
    if (params.length > minLength)
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
    params.painter->Add(params.order, textParams);
}

DotLine::DotLine(const LineParams& params)
    : DistanceLine(params)
{
}

void DotLine::Draw()
{
    using namespace DAVA;

    Painting::DrawLineParams lineParams;

    DistanceSystemPreferences* preferences = params.accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    lineParams.color = preferences->linesColor;
    lineParams.startPos = params.startPoint;
    lineParams.endPos = params.endPoint;
    lineParams.type = Painting::DrawLineParams::DOT;
    params.gd.BuildTransformMatrix(lineParams.transformMatrix);

    params.painter->Add(params.order, lineParams);
}
