#include "Physics/CollisionSingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>

#include <algorithm>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(CollisionSingleComponent)
{
    ReflectionRegistrator<CollisionSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

void CollisionSingleComponent::RemoveCollisionsWithEntity(Entity* entity)
{
    collisions.erase(std::remove_if(collisions.begin(),
                                    collisions.end(),
                                    [entity](const CollisionInfo& i) { return i.first == entity || i.second == entity; }),
                     collisions.end());
}

Vector<CollisionInfo> CollisionSingleComponent::GetCollisionsWithEntity(Entity* entity) const
{
    Vector<CollisionInfo> result;
    for (size_t i = 0; i < collisions.size(); ++i)
    {
        const CollisionInfo& info = collisions[i];
        if (info.first == entity || info.second == entity)
        {
            result.push_back(info);
        }
    }

    return result;
}

void CollisionSingleComponent::GetCollisionEntities(Entity* entity, UnorderedSet<Entity*>& result) const
{
    for (const CollisionInfo& info : collisions)
    {
        if (info.first == entity && entity != info.second)
        {
            result.insert(info.second);
        }
        if (info.second == entity && entity != info.first)
        {
            result.insert(info.first);
        }
    }
}

void CollisionSingleComponent::Clear()
{
    collisions.clear();
    activeTriggers.clear();
}

void CollisionSingleComponent::GetEntitesByTrigger(Entity* trigger, UnorderedSet<Entity*>& result) const
{
    for (const TriggerInfo& ti : activeTriggers)
    {
        if (ti.trigger == trigger)
        {
            result.insert(ti.other);
        }
    }
}

void CollisionSingleComponent::GetTriggersByEntity(Entity* entity, UnorderedSet<Entity*>& result) const
{
    for (const TriggerInfo& ti : activeTriggers)
    {
        if (ti.other == entity)
        {
            result.insert(ti.trigger);
        }
    }
}
}
