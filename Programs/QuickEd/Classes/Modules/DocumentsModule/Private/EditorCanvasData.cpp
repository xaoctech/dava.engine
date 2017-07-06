#include "Modules/DocumentsModule/EditorCanvasData.h"
#include "Modules/DocumentsModule/CentralWidgetData.h"

#include <TArc/Core/ContextAccessor.h>

#include <Base/FastName.h>
#include <Math/Vector.h>
#include <Logger/Logger.h>

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
    .Field(referencePointPropertyName.c_str(), &EditorCanvasData::referencePoint)
    .Field(movableControlPositionPropertyName.c_str(), &EditorCanvasData::GetMovableControlPosition, nullptr)
    .End();
}

EditorCanvasData::EditorCanvasData(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
{
    predefinedScales = { 0.01f, 0.02f, 0.04f, 0.08f, 0.16f,
                         0.32f, 0.64f, 1.0f, 2.00f, 4.00f,
                         8.00f, 16.0f, 24.0f, 32.0f };
}

DAVA::Vector2 EditorCanvasData::GetWorkAreaSize() const
{
    return workAreaSize;
}

void EditorCanvasData::SetWorkAreaSize(const DAVA::Vector2& size)
{
    DVASSERT(size.dx >= 0.0f && size.dy >= 0.0f);
    workAreaSize = size;
    if (workAreaSize.IsZero())
    {
        needCentralize = true;
    }
}

DAVA::Vector2 EditorCanvasData::GetCanvasSize() const
{
    return workAreaSize * scale + DAVA::Vector2(margin, margin) * 2.0f;
}

DAVA::Vector2 EditorCanvasData::GetPosition() const
{
    if (needCentralize)
    {
        return GetMaximumPosition() / 2.0f;
    }
    else
    {
        return position;
    }
}

void EditorCanvasData::SetPosition(const DAVA::Vector2& position_)
{
    using namespace DAVA;
    needCentralize = false;

    Vector2 minPos = GetMinimumPosition();
    Vector2 maxPos = GetMaximumPosition();

    //wheel and gesture events doesn't check min and max position
    position.Set(Clamp(position_.x, minPos.x, maxPos.x),
                 Clamp(position_.y, minPos.y, maxPos.y));
    position.Set(std::floor(position.x), std::floor(position.y));
}

DAVA::Vector2 EditorCanvasData::GetMinimumPosition() const
{
    return DAVA::Vector2(0.0f, 0.0f);
}

DAVA::Vector2 EditorCanvasData::GetMaximumPosition() const
{
    DAVA::Vector2 sizeDiff = GetCanvasSize() - GetViewSize();
    return DAVA::Vector2(std::max(0.0f, sizeDiff.dx), std::max(0.0f, sizeDiff.dy));
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
    return (GetMovableControlPosition() + GetRootPosition() * GetScale()) * -1;
}

DAVA::Vector2 EditorCanvasData::GetLastValue() const
{
    return GetStartValue() + GetViewSize();
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
    SetPosition((referencePoint + position - Vector2(margin, margin)) * scaleDiff - referencePoint + Vector2(margin, margin));

    //clear reference point before next call
    referencePoint = invalidPoint;
}

const DAVA::Vector<DAVA::float32>& EditorCanvasData::GetPredefinedScales() const
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
            movableControlPosition[axis] = (viewSize[axis] - workAreaSize[axis] * scale) / 2.0f;
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
