#include "Classes/Modules/CanvasModule/CanvasDataAdapter.h"
#include "Classes/Modules/CanvasModule/CanvasData.h"
#include "Classes/UI/Preview/Data/CentralWidgetData.h"

#include <TArc/Core/ContextAccessor.h>

#include <Reflection/ReflectedTypeDB.h>

DAVA::FastName CanvasDataAdapter::scalePropertyName{ "scale" };
DAVA::FastName CanvasDataAdapter::positionPropertyName{ "position" };
DAVA::FastName CanvasDataAdapter::minimumPositionPropertyName{ "minimum position" };
DAVA::FastName CanvasDataAdapter::maximumPositionPropertyName{ "maximum position" };
DAVA::FastName CanvasDataAdapter::startValuePropertyName{ "start value" };
DAVA::FastName CanvasDataAdapter::lastValuePropertyName{ "last value" };
DAVA::FastName CanvasDataAdapter::movableControlPositionPropertyName{ "movable control position" };

DAVA_VIRTUAL_REFLECTION_IMPL(CanvasDataAdapter)
{
    DAVA::ReflectionRegistrator<CanvasDataAdapter>::Begin()
    .Field(scalePropertyName.c_str(), &CanvasDataAdapter::GetScale, static_cast<void (CanvasDataAdapter::*)(DAVA::float32)>(&CanvasDataAdapter::SetScale))
    .Field(positionPropertyName.c_str(), &CanvasDataAdapter::GetPosition, &CanvasDataAdapter::SetPosition)
    .Field(minimumPositionPropertyName.c_str(), &CanvasDataAdapter::GetMinimumPosition, nullptr)
    .Field(maximumPositionPropertyName.c_str(), &CanvasDataAdapter::GetMaximumPosition, nullptr)
    .Field(startValuePropertyName.c_str(), &CanvasDataAdapter::GetStartValue, nullptr)
    .Field(lastValuePropertyName.c_str(), &CanvasDataAdapter::GetLastValue, nullptr)
    .Field(movableControlPositionPropertyName.c_str(), &CanvasDataAdapter::GetMovableControlPosition, nullptr)
    .End();
}

CanvasDataAdapter::CanvasDataAdapter(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
{
    DVASSERT(accessor != nullptr);
    canvasDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<CanvasData>());
}

CanvasDataAdapter::~CanvasDataAdapter() = default;

DAVA::Vector2 CanvasDataAdapter::GetPosition() const
{
    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return DAVA::Vector2(0.0f, 0.0f);
    }

    if (canvasData->IsCentralizeRequired())
    {
        return GetMaximumPosition() / 2.0f;
    }
    else
    {
        return canvasData->GetPosition();
    }
}

void CanvasDataAdapter::SetPosition(const DAVA::Vector2& pos)
{
    using namespace DAVA;

    Vector2 minPos = GetMinimumPosition();
    Vector2 maxPos = GetMaximumPosition();

    DAVA::Vector2 clampedPosition = DAVA::Vector2(Clamp(pos.x, minPos.x, maxPos.x),
                                                  Clamp(pos.y, minPos.y, maxPos.y));

    canvasDataWrapper.SetFieldValue(CanvasData::positionPropertyName, clampedPosition);
}

DAVA::Vector2 CanvasDataAdapter::GetMinimumPosition() const
{
    return DAVA::Vector2(0.0f, 0.0f);
}

DAVA::Vector2 CanvasDataAdapter::GetMaximumPosition() const
{
    using namespace DAVA;

    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return Vector2(0.0f, 0.0f);
    }

    Vector2 sizeDiff = canvasData->GetCanvasSize() - GetViewSize();
    return Vector2(std::max(0.0f, sizeDiff.dx), std::max(0.0f, sizeDiff.dy));
}

DAVA::Vector2 CanvasDataAdapter::GetStartValue() const
{
    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return DAVA::Vector2(0.0f, 0.0f);
    }

    DAVA::Vector2 startValue = (GetMovableControlPosition() + canvasData->GetRootPosition() * GetScale()) * -1;
    return DAVA::Vector2(startValue.x, startValue.y);
}

DAVA::Vector2 CanvasDataAdapter::GetLastValue() const
{
    return GetStartValue() + GetViewSize();
}

DAVA::float32 CanvasDataAdapter::GetScale() const
{
    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return 1.0f;
    }

    return canvasData->GetScale();
}

void CanvasDataAdapter::SetScale(DAVA::float32 scale)
{
    SetScale(scale, GetViewSize() / 2.0f);
}

void CanvasDataAdapter::SetScale(DAVA::float32 scale, const DAVA::Vector2& referencePoint)
{
    using namespace DAVA;

    const CanvasData* canvasData = GetCanvasData();
    DVASSERT(canvasData != nullptr);

    const Vector<float32>& predefinedScales = canvasData->GetPredefinedScales();

    float32 margin = canvasData->GetMargin();
    Vector2 position = Vector2(margin, margin) - GetMovableControlPosition();

    scale = Clamp(scale, predefinedScales.front(), predefinedScales.back());
    DVASSERT(scale != 0.0f);
    float32 scaleDiff = scale / canvasData->GetScale();

    canvasDataWrapper.SetFieldValue(CanvasData::scalePropertyName, scale);

    //recalculate new position to keep referncePoint on the same visible pos
    Vector2 newPosition = (referencePoint + position - Vector2(margin, margin)) * scaleDiff - referencePoint + Vector2(margin, margin);
    SetPosition(newPosition);
}

DAVA::Vector2 CanvasDataAdapter::GetMovableControlPosition() const
{
    using namespace DAVA;

    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return DAVA::Vector2(0.0f, 0.0f);
    }

    Vector2 canvasSize = canvasData->GetCanvasSize();
    Vector2 viewSize = GetViewSize();
    Vector2 workAreaSize = canvasData->GetWorkAreaSize();
    float32 scale = canvasData->GetScale();
    float32 margin = canvasData->GetMargin();

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

    return Vector2(movableControlPosition.x, movableControlPosition.y);
}

DAVA::Vector2 CanvasDataAdapter::GetViewSize() const
{
    DAVA::TArc::DataContext* globalContext = accessor->GetGlobalContext();
    CentralWidgetData* centralWidgetData = globalContext->GetData<CentralWidgetData>();
    return centralWidgetData->GetViewSize();
}

const CanvasData* CanvasDataAdapter::GetCanvasData() const
{
    using namespace DAVA;
    using namespace TArc;

    DataContext* active = accessor->GetActiveContext();
    if (active == nullptr)
    {
        return nullptr;
    }

    return active->GetData<CanvasData>();
}

DAVA::float32 CanvasDataAdapter::RelativeValueToAbsoluteValue(DAVA::float32 relValue, DAVA::Vector2::eAxis axis) const
{
    using namespace DAVA;
    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return relValue;
    }

    float32 scale = canvasData->GetScale();
    Vector2 startValue = GetStartValue();
    return std::ceilf((startValue[axis] + relValue * scale) / scale);
}

DAVA::float32 CanvasDataAdapter::RelativeValueToPosition(DAVA::float32 relValue, DAVA::Vector2::eAxis axis) const
{
    DAVA::float32 absValue = RelativeValueToAbsoluteValue(relValue, axis);
    return AbsoluteValueToPosition(absValue, axis);
}

DAVA::float32 CanvasDataAdapter::AbsoluteValueToPosition(DAVA::float32 absValue, DAVA::Vector2::eAxis axis) const
{
    using namespace DAVA;
    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return absValue;
    }

    float32 scale = canvasData->GetScale();
    Vector2 startValue = GetStartValue();
    return absValue * scale - startValue[axis];
}

DAVA::float32 CanvasDataAdapter::PositionToAbsoluteValue(DAVA::float32 position, DAVA::Vector2::eAxis axis) const
{
    using namespace DAVA;
    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return position;
    }

    float32 scale = canvasData->GetScale();
    Vector2 startValue = GetStartValue();
    return std::round((startValue[axis] + position) / scale);
}
