#include "PlayerInvaderComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;
REGISTER_CLASS(PlayerInvaderComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(PlayerInvaderComponent)
{
    ReflectionRegistrator<PlayerInvaderComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .End();
}

PlayerInvaderComponent::PlayerInvaderComponent()
{
}

Component* PlayerInvaderComponent::Clone(Entity* toEntity)
{
    PlayerInvaderComponent* component = new PlayerInvaderComponent();
    component->SetEntity(toEntity);
    return component;
}

PlayerInvaderComponent::~PlayerInvaderComponent()
{
}
