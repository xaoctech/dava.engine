#include "GameCollisionSingleComponent.h"
#include "Debug/DVAssert.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(GameCollisionSingleComponent)
{
    ReflectionRegistrator<GameCollisionSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

static const UnorderedSet<Entity*> EMPTY_SET = {};

void GameCollisionSingleComponent::SetCollision(Entity* entity1, Entity* entity2)
{
    DVASSERT(entity1 != entity2);
    collisions[entity1].insert(entity2);
    collisions[entity2].insert(entity1);
}
const UnorderedSet<Entity*>& GameCollisionSingleComponent::GetCollisions(Entity* entity) const
{
    const auto& it = collisions.find(entity);
    return (it == collisions.end()) ? EMPTY_SET : it->second;
}

void GameCollisionSingleComponent::Clear()
{
    collisions.clear();
}
