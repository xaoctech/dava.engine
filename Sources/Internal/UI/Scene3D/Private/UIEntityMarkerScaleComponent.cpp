#include "UI/Scene3D/UIEntityMarkerScaleComponent.h"
#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIEntityMarkerScaleComponent)
{
    ReflectionRegistrator<UIEntityMarkerScaleComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIEntityMarkerScaleComponent* c) { SafeRelease(c); })
    .Field("enabled", &UIEntityMarkerScaleComponent::IsEnabled, &UIEntityMarkerScaleComponent::SetEnabled)
    .Field("useAbsoluteDepth", &UIEntityMarkerScaleComponent::IsUseAbsoluteDepth, &UIEntityMarkerScaleComponent::SetUseAbsoluteDepth)
    .Field("depthToScaleFactor", &UIEntityMarkerScaleComponent::GetDepthToScaleFactor, &UIEntityMarkerScaleComponent::SetDepthToScaleFactor)
    .Field("maxScale", &UIEntityMarkerScaleComponent::GetMaxScale, &UIEntityMarkerScaleComponent::SetMaxScale)
    .Field("minScale", &UIEntityMarkerScaleComponent::GetMinScale, &UIEntityMarkerScaleComponent::SetMinScale)
    .End();
}

IMPLEMENT_UI_COMPONENT(UIEntityMarkerScaleComponent);

UIEntityMarkerScaleComponent::UIEntityMarkerScaleComponent() = default;

UIEntityMarkerScaleComponent::~UIEntityMarkerScaleComponent() = default;

UIEntityMarkerScaleComponent::UIEntityMarkerScaleComponent(const UIEntityMarkerScaleComponent& src)
    : enabled(src.enabled)
    , useAbsoluteDepth(src.useAbsoluteDepth)
    , depthToScaleFactor(src.depthToScaleFactor)
    , maxScale(src.maxScale)
    , minScale(src.minScale)
{
}

UIEntityMarkerScaleComponent* UIEntityMarkerScaleComponent::Clone() const
{
    return new UIEntityMarkerScaleComponent(*this);
}
}
