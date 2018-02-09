#pragma once

#include "Physics/PhysicsSystem.h"

namespace physx
{
class PxScene;
} // namespace physx

namespace DAVA
{
class PhysicsSystemPrivate
{
public:
    static physx::PxScene* GetPxScene(PhysicsSystem* system);
    static bool HasPendingComponents(PhysicsSystem* system);
};

inline physx::PxScene* PhysicsSystemPrivate::GetPxScene(PhysicsSystem* system)
{
    return system->physicsScene;
}

inline bool PhysicsSystemPrivate::HasPendingComponents(PhysicsSystem* system)
{
    bool hasPendingShape = false;
    system->ExecuteForEachPendingShape([&hasPendingShape](CollisionShapeComponent* s) { hasPendingShape = true; }); // TODO: change this when component group will support matchers
    return hasPendingShape;
}
} // namespace DAVA
