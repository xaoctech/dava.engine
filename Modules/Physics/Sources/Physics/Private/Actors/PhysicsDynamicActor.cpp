#include "Physics/Private/Actors/PhysicsDynamicActor.h"

#include <physx/PxRigidDynamic.h>

namespace DAVA
{
PhysicsDynamicActor::PhysicsDynamicActor(physx::PxRigidActor* actor)
    : PhysicsActor(actor)
{
    DVASSERT(actor->is<physx::PxRigidDynamic>() != nullptr);
}

} // namespace DAVA
