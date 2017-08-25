#include "EditorSystems/Private/DistanceLinesFactory.h"

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

ControlsLinesFactory::ControlsLinesFactory(const ControlLinesFactoryParams& params)
    : accessor(params.accessor)
    , font(params.font)
{
    using namespace DAVA;
    UIControl* highlightedControl = params.highlightedControl;
    UIControl* selectedControl = params.selectedControl;

    if (highlightedControl->GetParent() == selectedControl->GetParent())
    {
        selectedRect = selectedControl->GetRect();
        highlightedRect = highlightedControl->GetRect();
        parentGd = highlightedControl->GetParent()->GetGeometricData();
    }
    else if (highlightedControl->GetParent() == selectedControl)
    {
        selectedRect = Rect(Vector2(0.0f, 0.0f), selectedControl->GetSize());
        highlightedRect = highlightedControl->GetRect();
        parentGd = selectedControl->GetGeometricData();
    }
    else if (selectedControl->GetParent() == highlightedControl)
    {
        selectedRect = selectedControl->GetRect();
        highlightedRect = Rect(Vector2(0.0f, 0.0f), highlightedControl->GetSize());
        parentGd = highlightedControl->GetGeometricData();
    }
    else
    {
        DVASSERT("selected and highlighted nodes must be child and parent or be neighbours");
    }
}

DAVA::Vector<std::unique_ptr<DistanceLine>> ControlsLinesFactory::CreateLines() const
{
    using namespace DAVA;

    Vector<std::unique_ptr<DistanceLine>> lines;
    if (highlightedRect.RectIntersects(selectedRect))
    {
        for (int i = 0; i < Vector2::AXIS_COUNT; ++i)
        {
            Vector2::eAxis axis = static_cast<Vector2::eAxis>(i);
            Vector2::eAxis oppositeAxis = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;

            Vector2 startPos;
            Vector2 endPos;

            float32 selectedRectMiddle = selectedRect.GetPosition()[oppositeAxis] + selectedRect.GetSize()[oppositeAxis] / 2.0f;
            startPos[oppositeAxis] = selectedRectMiddle;
            endPos[oppositeAxis] = selectedRectMiddle;

            startPos[axis] = selectedRect.GetPosition()[axis];
            endPos[axis] = highlightedRect.GetPosition()[axis];
            AddSolidLine(axis, startPos, endPos, lines);

            startPos[axis] += selectedRect.GetSize()[axis];
            endPos[axis] += highlightedRect.GetSize()[axis];
            AddSolidLine(axis, startPos, endPos, lines);
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
            float32 highlightedRectMiddle = highlightedRect.GetPosition()[oppositeAxis] + highlightedRect.GetSize()[oppositeAxis] / 2.0f;
            startPos[oppositeAxis] = highlightedRectMiddle;
            endPos[oppositeAxis] = highlightedRectMiddle;

            if (highlightedRect.GetPosition()[axis] + highlightedRect.GetSize()[axis] < selectedRect.GetPosition()[axis])
            {
                startPos[axis] = highlightedRect.GetPosition()[axis] + highlightedRect.GetSize()[axis];
                endPos[axis] = selectedRect.GetPosition()[axis];
                AddSolidLine(axis, startPos, endPos, lines);
            }
            else if (highlightedRect.GetPosition()[axis] > selectedRect.GetPosition()[axis] + selectedRect.GetSize()[axis])
            {
                startPos[axis] = highlightedRect.GetPosition()[axis];
                endPos[axis] = selectedRect.GetPosition()[axis] + selectedRect.GetSize()[axis];
                AddSolidLine(axis, startPos, endPos, lines);
            }
            else
            {
                {
                    startPos[axis] = highlightedRect.GetPosition()[axis];
                    endPos[axis] = selectedRect.GetPosition()[axis];
                    AddSolidLine(axis, startPos, endPos, lines);
                }
                {
                    startPos[axis] = highlightedRect.GetPosition()[axis] + highlightedRect.GetSize()[axis];
                    endPos[axis] = selectedRect.GetPosition()[axis] + selectedRect.GetSize()[axis];
                    AddSolidLine(axis, startPos, endPos, lines);
                }
            }
        }
    }
    return lines;
}

SolidLine::SolidLineParams ControlsLinesFactory::CreateParams(const DAVA::Vector2& startPoint, const DAVA::Vector2& endPos, DAVA::eAlign direction) const
{
    SolidLine::SolidLineParams params(parentGd);
    params.accessor = accessor;
    params.font = font;
    params.startPoint = startPoint;
    params.endPoint = endPos;
    params.direction = direction;
    return params;
}

void ControlsLinesFactory::AddSolidLine(DAVA::Vector2::eAxis axis, const DAVA::Vector2& startPos, const DAVA::Vector2& endPos, DAVA::Vector<std::unique_ptr<DistanceLine>>& lines) const
{
    if (startPos[axis] != endPos[axis])
    {
        DAVA::eAlign direction = DistanceLinesFactoryDetails::GetDirection(axis, startPos, endPos);
        lines.push_back(std::make_unique<SolidLine>(CreateParams(startPos, endPos, direction)));
    }
}
