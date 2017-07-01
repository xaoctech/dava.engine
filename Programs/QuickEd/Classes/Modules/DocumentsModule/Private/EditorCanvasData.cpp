#include "Modules/DocumentsModule/EditorCanvasData.h"
#include "Modules/DocumentsModule/CentralWidgetData.h"

#include <TArc/Core/ContextAccessor.h>

#include <Base/FastName.h>
#include <Math/Vector.h>

DAVA::FastName EditorCanvasData::workAreaSizePropertyName{ "work area size" };
DAVA::FastName EditorCanvasData::canvasSizePropertyName{ "canvas size" };
DAVA::FastName EditorCanvasData::positionPropertyName{ "position" };
DAVA::FastName EditorCanvasData::minimumPositionPropertyName{ "minimum position" };
DAVA::FastName EditorCanvasData::maximumPositionPropertyName{ "maximum position" };
DAVA::FastName EditorCanvasData::rootPositionPropertyName{ "root control position" };
DAVA::FastName EditorCanvasData::startValuePropertyName{ "start value" };
DAVA::FastName EditorCanvasData::lastValuePropertyName{ "last value" };
DAVA::FastName EditorCanvasData::scalePropertyName{ "scale" };
DAVA::FastName EditorCanvasData::predefinedScalesPropertyName{ "predefined scales" };
DAVA::FastName EditorCanvasData::referencePointPropertyName{ "reference point" };
DAVA::FastName EditorCanvasData::movableControlPositionPropertyName{ "movable control position" };

DAVA_VIRTUAL_REFLECTION_IMPL(EditorCanvasData)
{
    DAVA::ReflectionRegistrator<EditorCanvasData>::Begin()
    .Field(workAreaSizePropertyName.c_str(), &EditorCanvasData::GetWorkAreaSize, &EditorCanvasData::SetWorkAreaSize)
    .Field(canvasSizePropertyName.c_str(), &EditorCanvasData::GetCanvasSize, nullptr)
    .Field(positionPropertyName.c_str(), &EditorCanvasData::GetPosition, &EditorCanvasData::SetPosition)
    .Field(minimumPositionPropertyName.c_str(), &EditorCanvasData::GetMinimumPosition, nullptr)
    .Field(maximumPositionPropertyName.c_str(), &EditorCanvasData::GetMaximumPosition, nullptr)
    .Field(rootPositionPropertyName.c_str(), &EditorCanvasData::GetRootPosition, &EditorCanvasData::SetRootPosition)
    .Field(startValuePropertyName.c_str(), &EditorCanvasData::GetStartValue, nullptr)
    .Field(lastValuePropertyName.c_str(), &EditorCanvasData::GetLastValue, nullptr)
    .Field(scalePropertyName.c_str(), &EditorCanvasData::GetScale, &EditorCanvasData::SetScale)
    .Field(predefinedScalesPropertyName.c_str(), &EditorCanvasData::GetPredefinedScales, nullptr)
    .Field(movableControlPositionPropertyName.c_str(), &EditorCanvasData::GetMovableControlPosition, nullptr)
    .End();
}

EditorCanvasData::EditorCanvasData(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
{
    predefinedScales = { 0.25f, 0.33f, 0.50f, 0.67f, 0.75f,
                         0.90f, 1.00f, 1.10f, 1.25f, 1.50f, 1.75f, 2.00f,
                         2.50f, 3.00f, 4.00f, 5.00f, 6.00f, 7.00f, 8.00f,
                         16.0f, 24.0f, 32.0f };
}

DAVA::Vector2 EditorCanvasData::GetWorkAreaSize() const
{
    return workAreaSize;
}

void EditorCanvasData::SetWorkAreaSize(const DAVA::Vector2& size)
{
    DVASSERT(size.dx >= 0.0f && size.dy >= 0.0f);
    workAreaSize = size;
}

DAVA::Vector2 EditorCanvasData::GetCanvasSize() const
{
    return workAreaSize * scale + DAVA::Vector2(margin, margin);
}

DAVA::Vector2 EditorCanvasData::GetPosition() const
{
    return position;
}

void EditorCanvasData::SetPosition(const DAVA::Vector2& position_)
{
    using namespace DAVA;
    Vector2 minPos = GetMinimumPosition();
    Vector2 maxPos = GetMaximumPosition();

    //wheel and gesture events doesn't check min and max position
    position.Set(Clamp(position_.x, minPos.x, maxPos.x),
                 Clamp(position_.y, minPos.y, maxPos.y));
}

DAVA::Vector2 EditorCanvasData::GetMinimumPosition() const
{
    return DAVA::Vector2(0.0f, 0.0f);
}

DAVA::Vector2 EditorCanvasData::GetMaximumPosition() const
{
    return GetCanvasSize() - GetViewSize();
}

DAVA::Vector2 EditorCanvasData::GetRootPosition() const
{
    return rootPosition;
}

void EditorCanvasData::SetRootPosition(const DAVA::Vector2& rootPosition_)
{
    DVASSERT(rootPosition_.dx >= 0.0f && rootPosition_.dy >= 0.0f);
    rootPosition = rootPosition_;
}

DAVA::Vector2 EditorCanvasData::GetStartValue() const
{
    return GetMovableControlPosition() + GetRootPosition() * GetScale();
}

DAVA::Vector2 EditorCanvasData::GetLastValue() const
{
    return GetStartValue() + GetViewSize() / GetScale();
}

DAVA::float32 EditorCanvasData::GetScale() const
{
    return scale;
}

void EditorCanvasData::SetScale(DAVA::float32 scale_)
{
    using namespace DAVA;
    scale_ = Clamp(scale_, predefinedScales.front(), predefinedScales.back());
    DVASSERT(scale != 0.0f);
    float32 scaleDiff = scale_ / scale;

    scale = scale_;

    Vector2 referenecePoint_ = referencePoint;
    if (referencePoint == invalidPoint)
    {
        referencePoint = GetViewSize() / 2.0f;
    }

    //recalculate new position to keep referncePoint on the same visible pos
    position = (referencePoint + position) * scaleDiff - referencePoint;

    //clear reference point before next call
    referencePoint = invalidPoint;
}

DAVA::Vector<DAVA::float32> EditorCanvasData::GetPredefinedScales() const
{
    return predefinedScales;
}

DAVA::Vector2 EditorCanvasData::GetMovableControlPosition() const
{
    using namespace DAVA;

    Vector2 canvasSize = GetCanvasSize();
    Vector2 viewSize = GetViewSize();

    Vector2 movableControlPosition;
    for (int i = 0; i < Vector2::AXIS_COUNT; ++i)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(i);
        if (canvasSize[axis] <= viewSize[axis])
        {
            movableControlPosition[axis] = (viewSize[axis] - canvasSize[axis]) / 2.0f;
        }
        else
        {
            movableControlPosition[axis] = margin - GetPosition()[axis];
        }
    }
    return movableControlPosition;
}

DAVA::Vector2 EditorCanvasData::GetViewSize() const
{
    DAVA::TArc::DataContext* globalContext = accessor->GetGlobalContext();
    CentralWidgetData* centralWidgetData = globalContext->GetData<CentralWidgetData>();
    return centralWidgetData->GetViewSize();
}

DAVA::float32 EditorCanvasData::GetNextScale(DAVA::int32 ticksCount) const
{
    using namespace DAVA;
    auto iter = std::upper_bound(predefinedScales.begin(), predefinedScales.end(), scale);
    if (iter == predefinedScales.end())
    {
        return scale;
    }
    ticksCount--;
    int32 distance = static_cast<int32>(std::distance(iter, predefinedScales.end()));
    ticksCount = std::min(distance, ticksCount);
    std::advance(iter, ticksCount);
    return iter != predefinedScales.end() ? *iter : predefinedScales.back();
}

DAVA::float32 EditorCanvasData::GetPreviousScale(DAVA::int32 ticksCount) const
{
    using namespace DAVA;
    auto iter = std::lower_bound(predefinedScales.begin(), predefinedScales.end(), scale);
    if (iter == predefinedScales.end())
    {
        return scale;
    }
    int32 distance = static_cast<int32>(std::distance(iter, predefinedScales.begin()));
    ticksCount = std::max(ticksCount, distance);
    std::advance(iter, ticksCount);
    return *iter;
}
