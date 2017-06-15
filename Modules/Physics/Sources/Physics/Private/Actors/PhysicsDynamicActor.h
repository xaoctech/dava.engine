#pragma once

#include "Physics/PhysicsActor.h"

namespace physx
{
class PxRigidDynamic;
} // namespace physx

namespace DAVA
{
class PhysicsDynamicActor : public PhysicsActor
{
public:
    PhysicsDynamicActor(physx::PxRigidActor* actor);
};
} // namespace DAVA
