#include "PlayerCharacterComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;
REGISTER_CLASS(PlayerCharacterComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(PlayerCharacterComponent)
{
    ReflectionRegistrator<PlayerCharacterComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .End();
}

PlayerCharacterComponent::PlayerCharacterComponent()
{
}

Component* PlayerCharacterComponent::Clone(Entity* toEntity)
{
    PlayerCharacterComponent* component = new PlayerCharacterComponent();
    component->SetEntity(toEntity);
    return component;
}

PlayerCharacterComponent::~PlayerCharacterComponent()
{
}
