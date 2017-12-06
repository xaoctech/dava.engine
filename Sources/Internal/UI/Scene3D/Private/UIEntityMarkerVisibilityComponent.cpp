#include "UI/Scene3D/UIEntityMarkerVisibilityComponent.h"
#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIEntityMarkerVisibilityComponent)
{
    ReflectionRegistrator<UIEntityMarkerVisibilityComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIEntityMarkerVisibilityComponent* c) { SafeRelease(c); })
    .Field("enabled", &UIEntityMarkerVisibilityComponent::IsEnabled, &UIEntityMarkerVisibilityComponent::SetEnabled)
    .End();
}

IMPLEMENT_UI_COMPONENT(UIEntityMarkerVisibilityComponent);

UIEntityMarkerVisibilityComponent::UIEntityMarkerVisibilityComponent() = default;

UIEntityMarkerVisibilityComponent::~UIEntityMarkerVisibilityComponent() = default;

UIEntityMarkerVisibilityComponent::UIEntityMarkerVisibilityComponent(const UIEntityMarkerVisibilityComponent& src)
    : enabled(src.enabled)
{
}

UIEntityMarkerVisibilityComponent* UIEntityMarkerVisibilityComponent::Clone() const
{
    return new UIEntityMarkerVisibilityComponent(*this);
}
}
