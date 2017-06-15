#pragma once

#include "Physics/PhysicsConfigs.h"

#include <ModuleManager/IModule.h>
#include <ModuleManager/ModuleManager.h>

#include <Base/BaseTypes.h>

namespace physx
{
class PxFoundation;
class PxPhysics;
class PxScene;
class PxRigidActor;
}

namespace DAVA
{
class PhysicsActor;
class Physics : public IModule
{
public:
    Physics(Engine* engine);

    void Init() override;
    void Shutdown() override;

    bool IsInitialized() const;

    physx::PxPhysics* GetPhysics() const;
    physx::PxFoundation* GetFoundation() const;
    physx::PxScene* CreateScene(const PhysicsSceneConfig& config) const;

    PhysicsActor* CloneActor(PhysicsActor* actor, void* userData) const;

    PhysicsActor* CreateStaticActor(void* userData) const;
    PhysicsActor* CreateDynamicActor(void* userData) const;

private:
    physx::PxRigidActor* ClonePxActor(physx::PxRigidActor* actor, void* userData) const;

private:
    physx::PxFoundation* foundation = nullptr;
    physx::PxPhysics* physics = nullptr;

    class PhysicsAllocator;
    PhysicsAllocator* allocator = nullptr;

    class PhysicsErrotCallback;
    PhysicsErrotCallback* errorCallback = nullptr;

    DAVA_VIRTUAL_REFLECTION(Physics, IModule);
};
}
