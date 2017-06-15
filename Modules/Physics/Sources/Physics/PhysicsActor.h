#pragma once

#include <Base/BaseObject.h>

namespace physx
{
class PxRigidActor;
} // namespace physx

namespace DAVA
{
class PhysicsActor : public BaseObject
{
public:
    PhysicsActor(physx::PxRigidActor* actor);
    virtual ~PhysicsActor();

    physx::PxRigidActor* GetPxActor() const;

protected:
    physx::PxRigidActor* actor;
};
} // namespace DAVA
