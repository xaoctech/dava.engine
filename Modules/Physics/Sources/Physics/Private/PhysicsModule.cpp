#include "Physics/PhysicsModule.h"
#include "Physics/PhysicsComponent.h"
#include "Physics/Private/PhysicsMath.h"
#include "Physics/Private/Actors/PhysicsStaticActor.h"
#include "Physics/Private/Actors/PhysicsDynamicActor.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Logger/Logger.h>
#include <MemoryManager/MemoryManager.h>
#include <Reflection/ReflectionRegistrator.h>

#include <physx/PxPhysicsAPI.h>
#include <PxShared/pvd/PxPvd.h>

namespace DAVA
{
namespace
{
physx::PxPvd* CreatePvd(physx::PxFoundation* foundation)
{
    IModule* physicsDebugModule = GetEngineContext()->moduleManager->GetModule("PhysicsDebug");
    if (physicsDebugModule == nullptr)
    {
        return nullptr;
    }

    Reflection moduleRef = Reflection::Create(ReflectedObject(physicsDebugModule));
    AnyFn fn = moduleRef.GetMethod("CreatePvd");
    DVASSERT(fn.IsValid() == true);
#if defined(__DAVAENGINE_DEBUG__)
    AnyFn::Params params = fn.GetInvokeParams();
    DVASSERT(params.retType == Type::Instance<physx::PxPvd*>());
    DVASSERT(params.argsType.size() == 1);
    DVASSERT(params.argsType[0] == Type::Instance<physx::PxFoundation*>());
#endif

    return fn.Invoke(foundation).Cast<physx::PxPvd*>(nullptr);
}

void ReleasePvd()
{
    IModule* physicsDebugModule = GetEngineContext()->moduleManager->GetModule("PhysicsDebug");
    if (physicsDebugModule == nullptr)
    {
        return;
    }

    Reflection moduleRef = Reflection::Create(ReflectedObject(physicsDebugModule));
    AnyFn fn = moduleRef.GetMethod("ReleasePvd");
    DVASSERT(fn.IsValid() == true);
    fn.Invoke();
}

void CopyBaseFields(physx::PxBase* src, physx::PxBase* dst)
{
    DVASSERT(src != nullptr);
    DVASSERT(dst != nullptr);

    DVASSERT(src->getConcreteType() == dst->getConcreteType());
    dst->setBaseFlags(src->getBaseFlags());
}

void CopyActorFields(physx::PxActor* src, physx::PxActor* dst)
{
    CopyBaseFields(src, dst);
    DVASSERT(src->getType() == dst->getType());

    dst->setName(src->getName());
    dst->setActorFlags(src->getActorFlags());
    dst->setDominanceGroup(src->getDominanceGroup());
    dst->setClientBehaviorFlags(src->getClientBehaviorFlags());
}

void CopyRigidBodyFields(physx::PxRigidBody* src, physx::PxRigidBody* dst)
{
    CopyActorFields(src, dst);

    dst->setCMassLocalPose(src->getCMassLocalPose());
    dst->setMass(src->getMass());
    dst->setMassSpaceInertiaTensor(src->getMassSpaceInertiaTensor());

    dst->setLinearVelocity(src->getLinearVelocity());
    dst->setAngularVelocity(src->getAngularVelocity());

    dst->setRigidBodyFlags(src->getRigidBodyFlags());
    dst->setMinCCDAdvanceCoefficient(src->getMinCCDAdvanceCoefficient());
    dst->setMaxDepenetrationVelocity(src->getMaxDepenetrationVelocity());
    dst->setMaxContactImpulse(src->getMaxContactImpulse());
}

void CopyRigidStaticFields(physx::PxRigidStatic* src, physx::PxRigidStatic* dst)
{
    CopyActorFields(src, dst);
}

void CopyRigidDynamicFields(physx::PxRigidDynamic* src, physx::PxRigidDynamic* dst)
{
    CopyRigidBodyFields(src, dst);

    dst->setLinearDamping(src->getLinearDamping());
    dst->setAngularDamping(src->getAngularDamping());
    dst->setMaxAngularVelocity(src->getMaxAngularVelocity());

    dst->setSleepThreshold(src->getSleepThreshold());
    dst->setStabilizationThreshold(src->getStabilizationThreshold());
    dst->setRigidDynamicLockFlags(src->getRigidDynamicLockFlags());
    if (src->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_SIMULATION) == false)
    {
        dst->setWakeCounter(src->getWakeCounter());
    }

    {
        physx::PxU32 minPositionIters = 1;
        physx::PxU32 minVelocityIters = 1;
        src->getSolverIterationCounts(minPositionIters, minVelocityIters);
        dst->setSolverIterationCounts(minPositionIters, minVelocityIters);
    }

    dst->setContactReportThreshold(src->getContactReportThreshold());
}
}

class Physics::PhysicsAllocator : public physx::PxAllocatorCallback
{
public:
    void* allocate(size_t size, const char* typeName, const char* filename, int line) override
    {
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
        return MemoryManager::Instance()->AlignedAllocate(size, 16, ALLOC_POOL_PHYSICS);
#else
        return defaultAllocator.allocate(size, typeName, filename, line);
#endif
    }

    void deallocate(void* ptr) override
    {
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
        MemoryManager::Instance()->Deallocate(ptr);
#else
        defaultAllocator.deallocate(ptr);
#endif
    }

private:
    physx::PxDefaultAllocator defaultAllocator;
};

class Physics::PhysicsErrotCallback : public physx::PxErrorCallback
{
public:
    void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
    {
        defaultErrorCallback.reportError(code, message, file, line);
    }

private:
    physx::PxDefaultErrorCallback defaultErrorCallback;
};

Physics::Physics(Engine* engine)
    : IModule(engine)
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Physics);
}

void Physics::Init()
{
    using namespace physx;

    allocator = new PhysicsAllocator();
    errorCallback = new PhysicsErrotCallback();

    foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, *allocator, *errorCallback);
    DVASSERT(foundation);

    physx::PxPvd* pvd = CreatePvd(foundation);
    physics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true, pvd);
    DVASSERT(physics);
    PxRegisterHeightFields(*physics);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PhysicsComponent);
}

void Physics::Shutdown()
{
    physics->release();
    ReleasePvd(); // PxPvd should be released between PxPhysics and PxFoundation
    foundation->release();
    SafeDelete(allocator);
    SafeDelete(errorCallback);
}

bool Physics::IsInitialized() const
{
    return foundation != nullptr && physics != nullptr;
}

physx::PxPhysics* Physics::GetPhysics() const
{
    return physics;
}

physx::PxFoundation* Physics::GetFoundation() const
{
    return foundation;
}

physx::PxScene* Physics::CreateScene(const PhysicsSceneConfig& config) const
{
    using namespace physx;

    DVASSERT(physics != nullptr);

    PxSceneDesc sceneDesc(physics->getTolerancesScale());
    sceneDesc.flags = PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    sceneDesc.gravity = PhysicsMath::Vector3ToPxVec3(config.gravity);

    PxDefaultCpuDispatcher* cpuDispatcher = PxDefaultCpuDispatcherCreate(config.threadCount);
    DVASSERT(cpuDispatcher);
    sceneDesc.cpuDispatcher = cpuDispatcher;

    sceneDesc.filterShader = PxDefaultSimulationFilterShader;

    PxScene* scene = physics->createScene(sceneDesc);
    DVASSERT(scene);

    return scene;
}

PhysicsActor* Physics::CloneActor(PhysicsActor* actor, void* userData) const
{
    if (actor == nullptr)
    {
        return nullptr;
    }

    physx::PxRigidActor* pxActor = actor->GetPxActor();
    PhysicsActor* resultActor = nullptr;
    switch (pxActor->getConcreteType())
    {
    case physx::PxConcreteType::eRIGID_STATIC:
        resultActor = new PhysicsStaticActor(ClonePxActor(pxActor, userData));
        break;
    case physx::PxConcreteType::eRIGID_DYNAMIC:
        resultActor = new PhysicsDynamicActor(ClonePxActor(pxActor, userData));
        break;
    default:
        DVASSERT(false);
        break;
    }
    return resultActor;
}

PhysicsActor* Physics::CreateStaticActor(void* userData) const
{
    physx::PxRigidStatic* actor = physics->createRigidStatic(physx::PxTransform(physx::PxIDENTITY::PxIdentity));
    actor->userData = userData;
    return new PhysicsStaticActor(actor);
}

PhysicsActor* Physics::CreateDynamicActor(void* userData) const
{
    physx::PxRigidDynamic* actor = physics->createRigidDynamic(physx::PxTransform(physx::PxIDENTITY::PxIdentity));
    actor->userData = userData;
    return new PhysicsDynamicActor(actor);
}

physx::PxRigidActor* Physics::ClonePxActor(physx::PxRigidActor* actor, void* userData) const
{
    DVASSERT(actor);

    physx::PxRigidActor* result = nullptr;

    switch (actor->getConcreteType())
    {
    case physx::PxConcreteType::eRIGID_STATIC:
    {
        physx::PxRigidStatic* staticActor = actor->is<physx::PxRigidStatic>();
        DVASSERT(staticActor != nullptr);

        physx::PxRigidStatic* resultStatic = physics->createRigidStatic(staticActor->getGlobalPose());
        CopyRigidStaticFields(staticActor, resultStatic);
        result = resultStatic;
    }
    break;
    case physx::PxConcreteType::eRIGID_DYNAMIC:
    {
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        DVASSERT(dynamicActor != nullptr);

        physx::PxRigidDynamic* resultStatic = physics->createRigidDynamic(dynamicActor->getGlobalPose());
        CopyRigidDynamicFields(dynamicActor, resultStatic);
        result = resultStatic;
    }
    break;
    default:
        DVASSERT(false);
        break;
    }

    result->userData = userData;
    return result;
}

DAVA_VIRTUAL_REFLECTION_IMPL(Physics)
{
    ReflectionRegistrator<Physics>::Begin()
    .End();
}
}
