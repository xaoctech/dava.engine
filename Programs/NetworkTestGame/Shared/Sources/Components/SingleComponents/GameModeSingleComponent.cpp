#include "GameModeSingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(GameModeSingleComponent)
{
    ReflectionRegistrator<GameModeSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

bool GameModeSingleComponent::IsMapLoaded() const
{
    return isMapLoaded;
}

void GameModeSingleComponent::SetIsMapLoaded(bool value)
{
    isMapLoaded = value;
}

DAVA::Entity* GameModeSingleComponent::GetPlayer() const
{
    return player;
}

void GameModeSingleComponent::SetPlayer(DAVA::Entity* entity)
{
    player = entity;
}

void GameModeSingleComponent::Clear()
{
}
