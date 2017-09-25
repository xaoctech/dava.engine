#include "Classes/Modules/DistanceLinesModule/Private/DistanceLinesFactory.h"

#include <UI/UIControl.h>

namespace DistanceLinesFactoryDetails
{
DAVA::eAlign GetDirection(DAVA::Vector2::eAxis axis, const DAVA::Vector2& startPos, const DAVA::Vector2& endPos)
{
    using namespace DAVA;
    if (axis == Vector2::AXIS_X)
    {
        return startPos.x < endPos.x ? eAlign::ALIGN_RIGHT : eAlign::ALIGN_LEFT;
    }
    else
    {
        return startPos.y < endPos.y ? eAlign::ALIGN_BOTTOM : eAlign::ALIGN_TOP;
    }
}
}

DAVA::Vector<std::unique_ptr<DistanceLine>> DistanceLinesFactory::CreateLines(const Params& params) const
{
    using namespace DAVA;

    Vector<std::unique_ptr<DistanceLine>> lines;
    if (params.highlightedRect.RectIntersects(params.selectedRect))
    {
        for (int i = 0; i < Vector2::AXIS_COUNT; ++i)
        {
            Vector2::eAxis axis = static_cast<Vector2::eAxis>(i);
            Vector2::eAxis oppositeAxis = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;

            Vector2 startPos;
            Vector2 endPos;

            float32 selectedRectMiddle = params.selectedRect.GetPosition()[oppositeAxis] + params.selectedRect.GetSize()[oppositeAxis] / 2.0f;
            startPos[oppositeAxis] = selectedRectMiddle;
            endPos[oppositeAxis] = selectedRectMiddle;

            startPos[axis] = params.selectedRect.GetPosition()[axis];
            endPos[axis] = params.highlightedRect.GetPosition()[axis];
            AddLine<SolidLine>(params, axis, startPos, endPos, lines);

            startPos[axis] += params.selectedRect.GetSize()[axis];
            endPos[axis] += params.highlightedRect.GetSize()[axis];
            AddLine<SolidLine>(params, axis, startPos, endPos, lines);
        }
    }
    else
    {
        for (int i = 0; i < Vector2::AXIS_COUNT; ++i)
        {
            Vector2::eAxis axis = static_cast<Vector2::eAxis>(i);
            Vector2::eAxis oppositeAxis = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;

            Vector2 startPos;
            Vector2 endPos;
            float32 selectedRectMiddle = params.selectedRect.GetPosition()[oppositeAxis] + params.selectedRect.GetSize()[oppositeAxis] / 2.0f;
            startPos[oppositeAxis] = selectedRectMiddle;
            endPos[oppositeAxis] = selectedRectMiddle;

            if (params.highlightedRect.GetPosition()[axis] + params.highlightedRect.GetSize()[axis] < params.selectedRect.GetPosition()[axis])
            { //if highlighted control at the left / top of the selected control

                startPos[axis] = params.selectedRect.GetPosition()[axis];
                endPos[axis] = params.highlightedRect.GetPosition()[axis] + params.highlightedRect.GetSize()[axis];
                AddLine<SolidLine>(params, axis, startPos, endPos, lines);
                SurroundWithDotLines(params, oppositeAxis, endPos, lines);
            }
            else if (params.highlightedRect.GetPosition()[axis] > params.selectedRect.GetPosition()[axis] + params.selectedRect.GetSize()[axis])
            {
                startPos[axis] = params.selectedRect.GetPosition()[axis] + params.selectedRect.GetSize()[axis];
                endPos[axis] = params.highlightedRect.GetPosition()[axis];
                AddLine<SolidLine>(params, axis, startPos, endPos, lines);
                SurroundWithDotLines(params, oppositeAxis, endPos, lines);
            }
            else
            {
                startPos[axis] = params.selectedRect.GetPosition()[axis];
                endPos[axis] = params.highlightedRect.GetPosition()[axis];
                AddLine<SolidLine>(params, axis, startPos, endPos, lines);
                SurroundWithDotLines(params, oppositeAxis, endPos, lines);

                startPos[axis] = params.selectedRect.GetPosition()[axis] + params.selectedRect.GetSize()[axis];
                endPos[axis] = params.highlightedRect.GetPosition()[axis] + params.highlightedRect.GetSize()[axis];
                AddLine<SolidLine>(params, axis, startPos, endPos, lines);
                SurroundWithDotLines(params, oppositeAxis, endPos, lines);
            }
        }
    }
    return lines;
}

LineParams DistanceLinesFactory::CreateLineParams(const Params& params, const DAVA::Vector2& startPoint, const DAVA::Vector2& endPos, DAVA::eAlign direction) const
{
    using namespace DAVA;

    LineParams lineParams(params.parentGd);
    lineParams.accessor = params.accessor;
    lineParams.startPoint = startPoint;
    lineParams.endPoint = endPos;
    lineParams.direction = direction;
    lineParams.axis = (direction == ALIGN_LEFT || direction == ALIGN_RIGHT) ? Vector2::AXIS_X : Vector2::AXIS_Y;
    lineParams.oppositeAxis = (lineParams.axis == Vector2::AXIS_X) ? Vector2::AXIS_Y : Vector2::AXIS_X;
    lineParams.order = params.order;
    lineParams.painter = params.painter;
    return lineParams;
}

void DistanceLinesFactory::SurroundWithDotLines(const Params& params, DAVA::Vector2::eAxis axis, const DAVA::Vector2& endPos, DAVA::Vector<std::unique_ptr<DistanceLine>>& lines) const
{
    using namespace DAVA;

    const DAVA::Rect& rect = params.highlightedRect;
    DAVA::Vector2 startPos = endPos;
    if (rect.GetPosition()[axis] + rect.GetSize()[axis] < endPos[axis])
    {
        startPos[axis] = rect.GetPosition()[axis] + rect.GetSize()[axis];
        AddLine<DotLine>(params, axis, startPos, endPos, lines);
    }
    else if (rect.GetPosition()[axis] > endPos[axis])
    {
        startPos[axis] = rect.GetPosition()[axis];
        AddLine<DotLine>(params, axis, startPos, endPos, lines);
    }
}

template <typename T>
void DistanceLinesFactory::AddLine(const Params& params, DAVA::Vector2::eAxis axis, const DAVA::Vector2& startPos, const DAVA::Vector2& endPos, DAVA::Vector<std::unique_ptr<DistanceLine>>& lines) const
{
    if (startPos[axis] != endPos[axis])
    {
        DAVA::eAlign direction = DistanceLinesFactoryDetails::GetDirection(axis, startPos, endPos);
        lines.push_back(std::make_unique<T>(CreateLineParams(params, startPos, endPos, direction)));
    }
}

DistanceLinesFactory::Params::Params(DAVA::UIControl* selectedControl, DAVA::UIControl* highlightedControl)
{
    using namespace DAVA;
    if (highlightedControl->GetParent() == selectedControl->GetParent())
    {
        selectedRect = selectedControl->GetLocalGeometricData().GetAABBox();
        highlightedRect = highlightedControl->GetLocalGeometricData().GetAABBox();
        parentGd = highlightedControl->GetParent()->GetGeometricData();
    }
    else if (highlightedControl->GetParent() == selectedControl)
    {
        selectedRect = Rect(Vector2(0.0f, 0.0f), selectedControl->GetSize());
        highlightedRect = highlightedControl->GetLocalGeometricData().GetAABBox();
        parentGd = selectedControl->GetGeometricData();
    }
    else if (selectedControl->GetParent() == highlightedControl)
    {
        selectedRect = selectedControl->GetLocalGeometricData().GetAABBox();
        highlightedRect = Rect(Vector2(0.0f, 0.0f), highlightedControl->GetSize());
        parentGd = highlightedControl->GetGeometricData();
    }
    else
    {
        DVASSERT("selected and highlighted nodes must be child and parent or be neighbours");
    }
}
