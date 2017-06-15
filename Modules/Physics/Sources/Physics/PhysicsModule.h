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
class PxActor;
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

    physx::PxActor* ClonePxActor(physx::PxActor* actor, void* userData) const;
    physx::PxActor* CreateStaticActor() const;
    physx::PxActor* CreateDynamicActor() const;

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
