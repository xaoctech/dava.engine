#include "Physics/PhysicsModule.h"
#include "Physics/StaticBodyComponent.h"
#include "Physics/DynamicBodyComponent.h"
#include "Physics/CollisionComponent.h"
#include "Physics/Private/PhysicsMath.h"

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

class PhysicsModule::PhysicsAllocator : public physx::PxAllocatorCallback
{
public:
    void* allocate(size_t size, const char* typeName, const char* filename, int line) override
    {
// MemoryManager temporary disabled as AlignedAllocate produce heap corruption on Deallocation
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
//return MemoryManager::Instance()->AlignedAllocate(size, 16, ALLOC_POOL_PHYSICS);
#else
#endif
        return defaultAllocator.allocate(size, typeName, filename, line);
    }

    void deallocate(void* ptr) override
    {
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
//MemoryManager::Instance()->Deallocate(ptr);
#else
#endif
        defaultAllocator.deallocate(ptr);
    }

private:
    physx::PxDefaultAllocator defaultAllocator;
};

class PhysicsModule::PhysicsErrotCallback : public physx::PxErrorCallback
{
public:
    void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
    {
        defaultErrorCallback.reportError(code, message, file, line);
    }

private:
    physx::PxDefaultErrorCallback defaultErrorCallback;
};

PhysicsModule::PhysicsModule(Engine* engine)
    : IModule(engine)
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PhysicsModule);
}

void PhysicsModule::Init()
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
    PxRegisterParticles(*physics); // For correct rigidDynamic->setGloblaPose after simulation stop

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StaticBodyComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(DynamicBodyComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CollisionComponent);
}

void PhysicsModule::Shutdown()
{
    physics->release();
    ReleasePvd(); // PxPvd should be released between PxPhysics and PxFoundation
    foundation->release();
    SafeDelete(allocator);
    SafeDelete(errorCallback);
}

bool PhysicsModule::IsInitialized() const
{
    return foundation != nullptr && physics != nullptr;
}

void* PhysicsModule::Allocate(size_t size, const char* typeName, const char* filename, int line)
{
    return allocator->allocate(size, typeName, filename, line);
}

void PhysicsModule::Deallocate(void* ptr)
{
    allocator->deallocate(ptr);
}

physx::PxScene* PhysicsModule::CreateScene(const PhysicsSceneConfig& config) const
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

physx::PxActor* PhysicsModule::ClonePxActor(physx::PxActor* actor, void* userData) const
{
    DVASSERT(actor);

    physx::PxActor* result = nullptr;

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

physx::PxActor* PhysicsModule::CreateStaticActor() const
{
    return physics->createRigidStatic(physx::PxTransform(physx::PxIDENTITY::PxIdentity));
}

physx::PxActor* PhysicsModule::CreateDynamicActor() const
{
    return physics->createRigidDynamic(physx::PxTransform(physx::PxIDENTITY::PxIdentity));
}

physx::PxShape* PhysicsModule::CreateBoxShape(bool exclusive) const
{
    return physics->createShape(physx::PxBoxGeometry(10.0f, 10.0f, 10.0f), *GetDefaultMaterial(), exclusive);
}

physx::PxMaterial* PhysicsModule::GetDefaultMaterial() const
{
    if (defaultMaterial == nullptr)
    {
        defaultMaterial = physics->createMaterial(0.5f, 0.5f, 1.f);
    }

    return defaultMaterial;
}

DAVA_VIRTUAL_REFLECTION_IMPL(PhysicsModule)
{
    ReflectionRegistrator<PhysicsModule>::Begin()
    .End();
}
}
