#pragma once

#include "Physics/PhysicsActor.h"

namespace physx
{
class PxRigidStatic;
} // namespace physx

namespace DAVA
{
class PhysicsStaticActor : public PhysicsActor
{
public:
    PhysicsStaticActor(physx::PxRigidActor* actor);
};
} // namespace DAVA