#include "PlayerCarComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;
REGISTER_CLASS(PlayerCarComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(PlayerCarComponent)
{
    ReflectionRegistrator<PlayerCarComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .End();
}

PlayerCarComponent::PlayerCarComponent()
{
}

Component* PlayerCarComponent::Clone(Entity* toEntity)
{
    PlayerCarComponent* component = new PlayerCarComponent();
    component->SetEntity(toEntity);
    return component;
}

PlayerCarComponent::~PlayerCarComponent()
{
}
