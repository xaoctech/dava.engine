#include "GameStunningComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;
REGISTER_CLASS(GameStunningComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(GameStunningComponent)
{
    ReflectionRegistrator<GameStunningComponent>::Begin()[M::Replicable(M::Privacy::PRIVATE)]
    .ConstructorByPointer()
    .End();
}

GameStunningComponent::GameStunningComponent()
{
}

Component* GameStunningComponent::Clone(Entity* toEntity)
{
    GameStunningComponent* component = new GameStunningComponent();
    component->SetEntity(toEntity);
    return component;
}

GameStunningComponent::~GameStunningComponent()
{
}
