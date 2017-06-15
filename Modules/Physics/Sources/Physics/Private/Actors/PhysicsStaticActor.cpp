#include "Physics/Private/Actors/PhysicsStaticActor.h"

#include <Debug/DVAssert.h>

#include <physx/PxRigidStatic.h>

namespace DAVA
{
PhysicsStaticActor::PhysicsStaticActor(physx::PxRigidActor* actor)
    : PhysicsActor(actor)
{
    DVASSERT(actor->is<physx::PxRigidStatic>() != nullptr);
}

} // namespace DAVA