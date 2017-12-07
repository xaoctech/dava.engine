#include "UI/Scene3D/UIEntityMarkerComponent.h"
#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"

ENUM_DECLARE(DAVA::UIEntityMarkerComponent::OrderMode)
{
    ENUM_ADD_DESCR(DAVA::UIEntityMarkerComponent::OrderMode::NearFront, "NearFront");
    ENUM_ADD_DESCR(DAVA::UIEntityMarkerComponent::OrderMode::NearBack, "NearBack");
}

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIEntityMarkerComponent)
{
    ReflectionRegistrator<UIEntityMarkerComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIEntityMarkerComponent* c) { SafeRelease(c); })
    .Field("targetEntity", &UIEntityMarkerComponent::GetTargetEntity, &UIEntityMarkerComponent::SetTargetEntity)[M::HiddenField()]
    .Field("enabled", &UIEntityMarkerComponent::IsEnabled, &UIEntityMarkerComponent::SetEnabled)
    .Field("syncVisibilityEnabled", &UIEntityMarkerComponent::IsSyncVisibilityEnabled, &UIEntityMarkerComponent::SetSyncVisibilityEnabled)
    .Field("syncPositionEnabled", &UIEntityMarkerComponent::IsSyncPositionEnabled, &UIEntityMarkerComponent::SetSyncPositionEnabled)
    .Field("syncScaleEnabled", &UIEntityMarkerComponent::IsSyncScaleEnabled, &UIEntityMarkerComponent::SetSyncScaleEnabled)
    .Field("scaleFactor", &UIEntityMarkerComponent::GetScaleFactor, &UIEntityMarkerComponent::SetScaleFactor)
    .Field("maxScale", &UIEntityMarkerComponent::GetMaxScale, &UIEntityMarkerComponent::SetMaxScale)
    .Field("minScale", &UIEntityMarkerComponent::GetMinScale, &UIEntityMarkerComponent::SetMinScale)
    .Field("syncOrderEnabled", &UIEntityMarkerComponent::IsSyncOrderEnabled, &UIEntityMarkerComponent::SetSyncOrderEnabled)
    .Field("orderMode", &UIEntityMarkerComponent::GetOrderMode, &UIEntityMarkerComponent::SetOrderMode)[M::EnumT<OrderMode>()]
    .Field("useCustomStrategy", &UIEntityMarkerComponent::IsUseCustomStrategy, &UIEntityMarkerComponent::SetUseCustomStrategy)
    .Field("customStrategy", &UIEntityMarkerComponent::GetCustomStrategy, &UIEntityMarkerComponent::SetCustomStrategy)[M::HiddenField()]
    .End();
}

IMPLEMENT_UI_COMPONENT(UIEntityMarkerComponent);

UIEntityMarkerComponent::UIEntityMarkerComponent() = default;

UIEntityMarkerComponent::~UIEntityMarkerComponent() = default;

UIEntityMarkerComponent::UIEntityMarkerComponent(const UIEntityMarkerComponent& src)
    : targetEntity(src.targetEntity)
    , enabled(src.enabled)
    , syncVisibilityEnabled(src.syncVisibilityEnabled)
    , syncPositionEnabled(src.syncPositionEnabled)
    , syncScaleEnabled(src.syncScaleEnabled)
    , scaleFactor(src.scaleFactor)
    , minScale(src.minScale)
    , maxScale(src.maxScale)
    , syncOrderEnabled(src.syncOrderEnabled)
    , orderMode(src.orderMode)
    , useCustomStrategy(src.useCustomStrategy)
    , customStrategy(src.customStrategy)
{
}

UIEntityMarkerComponent* UIEntityMarkerComponent::Clone() const
{
    return new UIEntityMarkerComponent(*this);
}
}
