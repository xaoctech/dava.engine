#include "Scene3D/Components/SingleComponents/CollisionSingleComponent.h"
#include "Scene3D/Entity.h"
#include "Reflection/ReflectionRegistrator.h"

#include <algorithm>

namespace DAVA
{
DAVA_REFLECTION_IMPL(CollisionPoint)
{
    ReflectionRegistrator<CollisionPoint>::Begin()
    .Field("position", &CollisionPoint::position)
    .Field("impulse", &CollisionPoint::impulse)
    .End();
}

DAVA_REFLECTION_IMPL(CollisionInfo)
{
    ReflectionRegistrator<CollisionInfo>::Begin()
    .Field("first", &CollisionInfo::first)
    .Field("second", &CollisionInfo::second)
    .Method("GetFirst", &CollisionInfo::GetFirst)
    .Method("GetSecond", &CollisionInfo::GetSecond)
    .End();
}

Entity* CollisionInfo::GetFirst() const
{
    return first;
}
Entity* CollisionInfo::GetSecond() const
{
    return second;
}

void CollisionSingleComponent::RemoveCollisionsWithEntity(Entity* entity)
{
    collisions.erase(std::remove_if(collisions.begin(),
                                    collisions.end(),
                                    [entity](const CollisionInfo& i) { return i.first == entity || i.second == entity; }),
                     collisions.end());
}
}
