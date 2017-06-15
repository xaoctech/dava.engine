#include "Physics/PhysicsActor.h"

#include <Debug/DVAssert.h>

#include <physx/PxRigidActor.h>

namespace DAVA
{
PhysicsActor::PhysicsActor(physx::PxRigidActor* actor_)
    : actor(actor_)
{
    DVASSERT(actor != nullptr);
}

PhysicsActor::~PhysicsActor()
{
    actor->release();
}

physx::PxRigidActor* PhysicsActor::GetPxActor() const
{
    return actor;
}

} // namespace DAVA
