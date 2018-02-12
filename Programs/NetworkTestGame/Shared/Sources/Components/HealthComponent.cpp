#include "HealthComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Logger/Logger.h>

using namespace DAVA;
REGISTER_CLASS(HealthComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(HealthComponent)
{
    ReflectionRegistrator<HealthComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .Field("Health", &HealthComponent::health)[M::Replicable()]
    .End();
}

HealthComponent::HealthComponent()
{
}

Component* HealthComponent::Clone(Entity* toEntity)
{
    HealthComponent* component = new HealthComponent();
    component->SetEntity(toEntity);
    return component;
}

void HealthComponent::DecHealth(uint8 dec, uint32 frameId)
{
    SetHealth(std::max(0, static_cast<int16>(GetHealth()) - static_cast<int16>(dec)));
    lastDamageFrameId = frameId;
}

void HealthComponent::SetHealth(DAVA::uint8 health_)
{
    health = health_;
}

uint8 HealthComponent::GetHealth() const
{
    return health;
}

uint32 HealthComponent::GetLastDamageId() const
{
    return lastDamageFrameId;
}

HealthComponent::~HealthComponent()
{
}
