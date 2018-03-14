#include "ExplosionEffectComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(ExplosionEffectComponent)
{
    ReflectionRegistrator<ExplosionEffectComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .Field("Duration", &ExplosionEffectComponent::duration)[M::Replicable()]
    .Field("EffectType", &ExplosionEffectComponent::effectType)[M::Replicable()]
    .End();
}

ExplosionEffectComponent::ExplosionEffectComponent() = default;

ExplosionEffectComponent::~ExplosionEffectComponent() = default;

DAVA::Component* ExplosionEffectComponent::Clone(DAVA::Entity* toEntity)
{
    ExplosionEffectComponent* component = new ExplosionEffectComponent;
    component->SetEntity(toEntity);
    return component;
}
