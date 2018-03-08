#include "PlayerTankComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;
REGISTER_CLASS(PlayerTankComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(PlayerTankComponent)
{
    ReflectionRegistrator<PlayerTankComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .End();
}

Component* PlayerTankComponent::Clone(Entity* toEntity)
{
    PlayerTankComponent* component = new PlayerTankComponent();
    component->SetEntity(toEntity);
    return component;
}
