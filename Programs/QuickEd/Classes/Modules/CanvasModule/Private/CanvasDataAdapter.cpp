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

DAVA_VIRTUAL_REFLECTION_IMPL(CanvasDataAdapter)
{
    DAVA::ReflectionRegistrator<CanvasDataAdapter>::Begin()
    .Field(scalePropertyName.c_str(), &CanvasDataAdapter::GetScale, static_cast<void (CanvasDataAdapter::*)(DAVA::float32)>(&CanvasDataAdapter::SetScale))
    .Field(positionPropertyName.c_str(), &CanvasDataAdapter::GetPosition, nullptr)
    .Field(minimumPositionPropertyName.c_str(), &CanvasDataAdapter::GetMinimumPosition, nullptr)
    .Field(maximumPositionPropertyName.c_str(), &CanvasDataAdapter::GetMaximumPosition, nullptr)
    .Field(startValuePropertyName.c_str(), &CanvasDataAdapter::GetStartValue, nullptr)
    .Field(lastValuePropertyName.c_str(), &CanvasDataAdapter::GetLastValue, nullptr)
    .End();
}

CanvasDataAdapter::CanvasDataAdapter(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
{
    DVASSERT(accessor != nullptr);
    canvasDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<CanvasData>());
}

CanvasDataAdapter::~CanvasDataAdapter() = default;

DAVA::Vector2 CanvasDataAdapter::GetDisplacementPosition() const
{
    using namespace DAVA;

    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return DAVA::Vector2(0.0f, 0.0f);
    }
    Vector2 topLeftOpverflow = GetTopLeftOverflow();
    Vector2 displacement = canvasData->GetDisplacement();
    return displacement + topLeftOpverflow;
}

void CanvasDataAdapter::SetDisplacementPosition(const DAVA::Vector2& position)
{
    using namespace DAVA;

    Vector2 topLeftOpverflow = GetTopLeftOverflow();
    canvasDataWrapper.SetFieldValue(CanvasData::displacementPropertyName, position - topLeftOpverflow);
}

void CanvasDataAdapter::SetDisplacementPositionSafe(const DAVA::Vector2& position)
{
    using namespace DAVA;

    Vector2 minPos = GetMinimumPosition();
    Vector2 maxPos = GetMaximumPosition();

    DAVA::Vector2 clamped = DAVA::Vector2(Clamp(position.x, minPos.x, maxPos.x),
                                          Clamp(position.y, minPos.y, maxPos.y));

    SetDisplacementPosition(clamped);
}

DAVA::Vector2 CanvasDataAdapter::GetPosition() const
{
    using namespace DAVA;

    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return Vector2(0.0f, 0.0f);
    }

    float32 scale = GetScale();
    Vector2 maximumPosition = GetMaximumPosition();
    Vector2 viewSize = GetViewSize();
    Vector2 rootSize = canvasData->GetRootControlSize() * scale;
    Vector2 displacement = canvasData->GetDisplacement();

    Vector2 position = (viewSize - rootSize) / 2.0f;
    return position - displacement;
}

DAVA::Vector2 CanvasDataAdapter::GetMinimumPosition() const
{
    return DAVA::Vector2(0.0f, 0.0f);
}

DAVA::Vector2 CanvasDataAdapter::GetMaximumPosition() const
{
    return GetTopLeftOverflow() + GetBottomRightOverflow();
}

DAVA::Vector2 CanvasDataAdapter::GetTopLeftOverflow() const
{
    using namespace DAVA;
    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return Vector2(0.0f, 0.0f);
    }

    float32 scale = GetScale();
    Vector2 rootPosition = canvasData->GetRootPosition() * scale;
    Vector2 rootSize = canvasData->GetRootControlSize() * scale;
    Vector2 margin = canvasData->GetMargin();
    Vector2 viewSize = GetViewSize();

    Vector2 workAreaTopLeft = rootPosition + rootSize / 2.0f + margin;
    DVASSERT(workAreaTopLeft.dx >= 0.0f && workAreaTopLeft.dy >= 0.0f);
    Vector2 topOverflow = workAreaTopLeft - viewSize / 2.0f;
    topOverflow.Set(std::max(0.0f, topOverflow.dx), std::max(0.0f, topOverflow.dy));
    return topOverflow;
}

DAVA::Vector2 CanvasDataAdapter::GetBottomRightOverflow() const
{
    using namespace DAVA;
    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return Vector2(0.0f, 0.0f);
    }

    float32 scale = GetScale();
    Vector2 rootPosition = canvasData->GetRootPosition() * scale;
    Vector2 rootSize = canvasData->GetRootControlSize() * scale;
    Vector2 margin = canvasData->GetMargin();
    Vector2 canvasSize = canvasData->GetCanvasSize();
    Vector2 workAreaBottomRight = canvasSize - rootPosition - rootSize / 2.0f - margin;
    Vector2 viewSize = GetViewSize();

    DVASSERT(workAreaBottomRight.dx >= 0.0f && workAreaBottomRight.dy >= 0.0f);

    Vector2 bottomOverflow = workAreaBottomRight - viewSize / 2.0f;
    bottomOverflow.Set(std::max(0.0f, bottomOverflow.dx), std::max(0.0f, bottomOverflow.dy));
    return bottomOverflow;
}

DAVA::Vector2 CanvasDataAdapter::GetStartValue() const
{
    using namespace DAVA;

    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return Vector2(0.0f, 0.0f);
    }

    return -1.0f * GetPosition();
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

    Vector2 margin = canvasData->GetMargin();
    Vector2 position = GetDisplacementPosition();

    scale = Clamp(scale, predefinedScales.front(), predefinedScales.back());
    DVASSERT(scale != 0.0f);
    float32 scaleDiff = scale / canvasData->GetScale();

    canvasDataWrapper.SetFieldValue(CanvasData::scalePropertyName, scale);

    //recalculate new position to keep referncePoint on the same visible pos
    Vector2 newPosition = (referencePoint + position - margin) * scaleDiff - referencePoint + margin;
    SetDisplacementPositionSafe(newPosition);
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

DAVA::Vector2 CanvasDataAdapter::MapFromRootToScreen(const DAVA::Vector2& absValue) const
{
    return DAVA::Vector2(MapFromRootToScreen(absValue.x, DAVA::Vector2::AXIS_X), MapFromRootToScreen(absValue.y, DAVA::Vector2::AXIS_Y));
}

DAVA::Vector2 CanvasDataAdapter::MapFromScreenToRoot(const DAVA::Vector2& position) const
{
    return DAVA::Vector2(MapFromScreenToRoot(position.x, DAVA::Vector2::AXIS_X), MapFromScreenToRoot(position.y, DAVA::Vector2::AXIS_Y));
}

DAVA::float32 CanvasDataAdapter::MapFromRootToScreen(DAVA::float32 absValue, DAVA::Vector2::eAxis axis) const
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

DAVA::float32 CanvasDataAdapter::MapFromScreenToRoot(DAVA::float32 position, DAVA::Vector2::eAxis axis) const
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
