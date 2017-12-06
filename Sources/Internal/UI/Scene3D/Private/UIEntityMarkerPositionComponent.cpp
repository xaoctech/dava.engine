#include "UI/Scene3D/UIEntityMarkerPositionComponent.h"
#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIEntityMarkerPositionComponent)
{
    ReflectionRegistrator<UIEntityMarkerPositionComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIEntityMarkerPositionComponent* c) { SafeRelease(c); })
    .Field("enabled", &UIEntityMarkerPositionComponent::IsEnabled, &UIEntityMarkerPositionComponent::SetEnabled)
    .End();
}

IMPLEMENT_UI_COMPONENT(UIEntityMarkerPositionComponent);

UIEntityMarkerPositionComponent::UIEntityMarkerPositionComponent() = default;

UIEntityMarkerPositionComponent::~UIEntityMarkerPositionComponent() = default;

UIEntityMarkerPositionComponent::UIEntityMarkerPositionComponent(const UIEntityMarkerPositionComponent& src)
    : enabled(src.enabled)
{
}

UIEntityMarkerPositionComponent* UIEntityMarkerPositionComponent::Clone() const
{
    return new UIEntityMarkerPositionComponent(*this);
}
}
