#include "GameStunnableComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;
REGISTER_CLASS(GameStunnableComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(GameStunnableComponent)
{
    ReflectionRegistrator<GameStunnableComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .Field("Cooldown", &GameStunnableComponent::cooldown)[M::Replicable()]
    .ConstructorByPointer()
    .End();
}

GameStunnableComponent::GameStunnableComponent()
{
}

Component* GameStunnableComponent::Clone(Entity* toEntity)
{
    GameStunnableComponent* component = new GameStunnableComponent();
    component->SetEntity(toEntity);
    return component;
}

void GameStunnableComponent::SetCooldown(float32 cooldown_)
{
    cooldown = cooldown_;
}

float32 GameStunnableComponent::GetCooldown() const
{
    return cooldown;
}

bool GameStunnableComponent::IsStunned() const
{
    return cooldown > 0.f;
}

GameStunnableComponent::~GameStunnableComponent()
{
}
