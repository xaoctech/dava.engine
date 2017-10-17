#include "UI/Preview/ScrollBarAdapter.h"

#include <TArc/Core/ContextAccessor.h>

#include <Reflection/ReflectedTypeDB.h>

DAVA::FastName ScrollBarAdapter::positionPropertyName{ "position" };
DAVA::FastName ScrollBarAdapter::minPosPropertyName{ "minimum position" };
DAVA::FastName ScrollBarAdapter::maxPosPropertyName{ "maximum position" };
DAVA::FastName ScrollBarAdapter::pageStepPropertyName{ "page step" };
DAVA::FastName ScrollBarAdapter::enabledPropertyName{ "enabled" };
DAVA::FastName ScrollBarAdapter::visiblePropertyName{ "visible" };
DAVA::FastName ScrollBarAdapter::orientationPropertyName{ "orientation" };

DAVA_REFLECTION_IMPL(ScrollBarAdapter)
{
    DAVA::ReflectionRegistrator<ScrollBarAdapter>::Begin()
    .Field(positionPropertyName.c_str(), &ScrollBarAdapter::GetPosition, &ScrollBarAdapter::SetPosition)
    .Field(minPosPropertyName.c_str(), &ScrollBarAdapter::GetMinPos, nullptr)
    .Field(maxPosPropertyName.c_str(), &ScrollBarAdapter::GetMaxPos, nullptr)
    .Field(pageStepPropertyName.c_str(), &ScrollBarAdapter::GetPageStep, nullptr)
    .Field(enabledPropertyName.c_str(), &ScrollBarAdapter::IsEnabled, nullptr)
    .Field(visiblePropertyName.c_str(), &ScrollBarAdapter::IsVisible, nullptr)
    .Field(orientationPropertyName.c_str(), &ScrollBarAdapter::GetOrientation, nullptr)
    .End();
}

ScrollBarAdapter::ScrollBarAdapter(DAVA::Vector2::eAxis orientation_, DAVA::TArc::ContextAccessor* accessor_)
    : orientation(orientation_)
    , accessor(accessor_)
    , canvasDataAdapter(accessor)
{
}

int ScrollBarAdapter::GetPosition() const
{
    return static_cast<int>(canvasDataAdapter.GetDisplacementPosition()[orientation]);
}

void ScrollBarAdapter::SetPosition(int value)
{
    DAVA::Vector2 displacement = canvasDataAdapter.GetDisplacementPosition();
    displacement[orientation] = value;
    canvasDataAdapter.SetDisplacementPosition(displacement);
}

int ScrollBarAdapter::GetMinPos() const
{
    return static_cast<int>(canvasDataAdapter.GetMinimumPosition()[orientation]);
}

int ScrollBarAdapter::GetMaxPos() const
{
    return static_cast<int>(canvasDataAdapter.GetMaximumPosition()[orientation]);
}

int ScrollBarAdapter::GetPageStep() const
{
    return canvasDataAdapter.GetViewSize()[orientation];
}

bool ScrollBarAdapter::IsEnabled() const
{
    return accessor->GetActiveContext() != nullptr;
}

bool ScrollBarAdapter::IsVisible() const
{
    return GetMaxPos() > 0;
}

Qt::Orientation ScrollBarAdapter::GetOrientation() const
{
    return orientation == DAVA::Vector2::AXIS_X ? Qt::Horizontal : Qt::Vertical;
}
