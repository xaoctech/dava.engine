#include "Physics/BoxCharacterControllerComponent.h"
#include "Physics/BoxShapeComponent.h"
#include "Physics/CapsuleCharacterControllerComponent.h"
#include "Physics/CapsuleShapeComponent.h"
#include "Physics/CollisionShapeComponent.h"
#include "Physics/CollisionSingleComponent.h"
#include "Physics/ConvexHullShapeComponent.h"
#include "Physics/HeightFieldShapeComponent.h"
#include "Physics/MeshShapeComponent.h"
#include "Physics/PhysicsComponent.h"
#include "Physics/PhysicsConfigs.h"
#include "Physics/PhysicsGeometryCache.h"
#include "Physics/PhysicsModule.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/PhysicsVehiclesSubsystem.h"
#include "Physics/PlaneShapeComponent.h"
#include "Physics/Private/PhysicsMath.h"
#include "Physics/SphereShapeComponent.h"
#include "Physics/VehicleCarComponent.h"
#include "Physics/VehicleTankComponent.h"

#include <Base/Type.h>
#include <Debug/ProfilerCPU.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Entity/Component.h>
#include <FileSystem/KeyedArchive.h>
#include <Logger/Logger.h>
#include <ModuleManager/ModuleManager.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/RenderHelper.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Components/SwitchComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Utils/Utils.h>

#include <PxShared/foundation/PxAllocatorCallback.h>
#include <PxShared/foundation/PxFoundation.h>

#include <physx/PxRigidActor.h>
#include <physx/PxRigidDynamic.h>
#include <physx/PxScene.h>
#include <physx/common/PxRenderBuffer.h>

#include <functional>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(PhysicsSystem)
{
    ReflectionRegistrator<PhysicsSystem>::Begin()[M::Tags("base", "physics")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixedFetch", &PhysicsSystem::ProcessFixedFetch)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 4.01f)]
    .Method("ProcessFixedSimulate", &PhysicsSystem::ProcessFixedSimulate)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 12.0f)]
    .End();
}

namespace PhysicsSystemDetail
{
template <typename T>
void EraseComponent(T* component, Vector<T*>& pendingComponents, Vector<T*>& components)
{
    auto addIter = std::find(pendingComponents.begin(), pendingComponents.end(), component);
    if (addIter != pendingComponents.end())
    {
        RemoveExchangingWithLast(pendingComponents, std::distance(pendingComponents.begin(), addIter));
    }
    else
    {
        auto iter = std::find(components.begin(), components.end(), component);
        if (iter != components.end())
        {
            RemoveExchangingWithLast(components, std::distance(components.begin(), iter));
        }
    }
}

void UpdateActorGlobalPose(physx::PxRigidActor* actor, const Vector3& position, const Quaternion& rotation)
{
    DVASSERT(actor != nullptr);

    physx::PxVec3 pxNewPos = PhysicsMath::Vector3ToPxVec3(position);
    physx::PxQuat pxNewQuat = PhysicsMath::QuaternionToPxQuat(rotation);
    const physx::PxTransform& pxCurrentTransform = actor->getGlobalPose();

    // Avoid updating global pose if it hasn't really changed to not wake up the actor
    if ((pxCurrentTransform.p != pxNewPos) || !(pxCurrentTransform.q == pxNewQuat))
    {
        actor->setGlobalPose(physx::PxTransform(pxNewPos, pxNewQuat));
    }
}

void UpdateShapeLocalPose(physx::PxShape* shape, const Vector3& position, const Quaternion& rotation)
{
    DVASSERT(shape != nullptr);

    physx::PxVec3 pxNewPos = PhysicsMath::Vector3ToPxVec3(position);
    physx::PxQuat pxNewQuat = PhysicsMath::QuaternionToPxQuat(rotation);
    const physx::PxTransform& pxCurrentTransform = shape->getLocalPose();

    // Avoid updating local pose if it hasn't really changed to not wake up the actor
    if ((pxCurrentTransform.p != pxNewPos) || !(pxCurrentTransform.q == pxNewQuat))
    {
        shape->setLocalPose(physx::PxTransform(pxNewPos, pxNewQuat));
    }
}

void UpdateShapeGeometryScale(physx::PxShape* shape, const Vector3& scale)
{
    DVASSERT(shape != nullptr);

    physx::PxVec3 pxNewScale = PhysicsMath::Vector3ToPxVec3(scale);

    physx::PxGeometryHolder geometryHolder = shape->getGeometry();
    physx::PxGeometryType::Enum geometryType = geometryHolder.getType();
    if (geometryType == physx::PxGeometryType::eTRIANGLEMESH)
    {
        physx::PxTriangleMeshGeometry geometry;
        bool extracted = shape->getTriangleMeshGeometry(geometry);
        DVASSERT(extracted);

        if (geometry.scale.scale != pxNewScale)
        {
            geometry.scale.scale = pxNewScale;
            shape->setGeometry(geometry);
        }
    }
    else if (geometryType == physx::PxGeometryType::eCONVEXMESH)
    {
        physx::PxConvexMeshGeometry geometry;
        bool extracted = shape->getConvexMeshGeometry(geometry);
        DVASSERT(extracted);

        if (geometry.scale.scale != pxNewScale)
        {
            geometry.scale.scale = pxNewScale;
            shape->setGeometry(geometry);
        }
    }
}

bool IsCollisionShapeType(const Type* componentType)
{
    PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    const Vector<const Type*>& shapeComponents = module->GetShapeComponentTypes();
    return std::any_of(shapeComponents.begin(), shapeComponents.end(), [componentType](const Type* type) {
        return componentType == type;
    });
}

bool IsCharacterControllerType(const Type* componentType)
{
    return componentType->Is<BoxCharacterControllerComponent>() || componentType->Is<CapsuleCharacterControllerComponent>();
}

Vector3 AccumulateMeshInfo(Entity* e, Vector<PolygonGroup*>& groups)
{
    RenderObject* ro = GetRenderObject(e);
    if (ro != nullptr)
    {
        uint32 batchesCount = ro->GetRenderBatchCount();
        int32 maxLod = ro->GetMaxLodIndex();
        for (uint32 i = 0; i < batchesCount; ++i)
        {
            int32 lodIndex = -1;
            int32 switchIndex = -1;
            RenderBatch* batch = ro->GetRenderBatch(i, lodIndex, switchIndex);
            if (lodIndex == maxLod)
            {
                PolygonGroup* group = batch->GetPolygonGroup();
                if (group != nullptr)
                {
                    groups.push_back(group);
                }
            }
        }
    }

    Vector3 pos;
    Vector3 scale;
    Quaternion quat;
    TransformComponent* transformComponent = GetTransformComponent(e);
    transformComponent->GetLocalTransform().Decomposition(pos, scale, quat);

    return scale;
}

PhysicsComponent* GetParentPhysicsComponent(Entity* entity)
{
    PhysicsComponent* physicsComponent = static_cast<PhysicsComponent*>(entity->GetComponent<StaticBodyComponent>());
    if (physicsComponent == nullptr)
    {
        physicsComponent = static_cast<PhysicsComponent*>(entity->GetComponent<DynamicBodyComponent>());
    }

    if (physicsComponent != nullptr)
    {
        return physicsComponent;
    }
    else
    {
        // Move up in the hierarchy
        Entity* parent = entity->GetParent();
        if (parent != nullptr)
        {
            return GetParentPhysicsComponent(parent);
        }
        else
        {
            return nullptr;
        }
    }
}

bool IsPhysicsEntity(Entity& entity, Component& componentToBeRemoved)
{
    PhysicsComponent* bodyComponent = PhysicsUtils::GetBodyComponent(&entity);
    if (bodyComponent != nullptr && bodyComponent != &componentToBeRemoved)
    {
        return true;
    }

    Vector<CollisionShapeComponent*> shapeComponents = PhysicsUtils::GetShapeComponents(&entity);
    if (shapeComponents.size() > 0)
    {
        if (shapeComponents.size() != 1 || shapeComponents[0] != &componentToBeRemoved)
        {
            return true;
        }
    }

    CharacterControllerComponent* cctComponent = PhysicsUtils::GetCharacterControllerComponent(&entity);
    if (cctComponent != nullptr && cctComponent != &componentToBeRemoved)
    {
        return true;
    }

    return false;
}

const uint32 DEFAULT_SIMULATION_BLOCK_SIZE = 16 * 1024 * 512;
} // namespace

physx::PxFilterFlags FilterShader(physx::PxFilterObjectAttributes attributes0,
                                  physx::PxFilterData filterData0,
                                  physx::PxFilterObjectAttributes attributes1,
                                  physx::PxFilterData filterData1,
                                  physx::PxPairFlags& pairFlags,
                                  const void* constantBlock,
                                  physx::PxU32 constantBlockSize)
{
    PX_UNUSED(attributes0);
    PX_UNUSED(attributes1);
    PX_UNUSED(constantBlockSize);
    PX_UNUSED(constantBlock);

    // PxFilterData for a shape is used this way:
    // - PxFilterData.word0 is used for engine-specific features (i.e. for CCD)
    // - PxFilterData.word1 is a bitmask for encoding type of object
    // - PxFilterData.word2 is a bitmask for encoding types of objects this object collides with
    // - PxFilterData.word3 is not used right now
    // Type of a shape and types it collides with can be set using CollisionShapeComponent::SetTypeMask and CollisionShapeComponent::SetTypeMaskToCollideWith methods

    if ((filterData0.word1 & filterData1.word2) == 0 &&
        (filterData1.word1 & filterData0.word2) == 0)
    {
        // If these types of objects do not collide, ignore this pair unless filter data for either of them changes
        return physx::PxFilterFlag::eSUPPRESS;
    }

    pairFlags =
    physx::PxPairFlag::eCONTACT_DEFAULT | // default collision processing
    physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | // notify about a first contact
    physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS | // notify about ongoing contacts
    physx::PxPairFlag::eNOTIFY_CONTACT_POINTS; // report contact points

    if (CollisionShapeComponent::IsCCDEnabled(filterData0) || CollisionShapeComponent::IsCCDEnabled(filterData1))
    {
        pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT; // report continuous collision detection contacts
    }

    return physx::PxFilterFlag::eDEFAULT;
}

class CCTQueryFilterCallback final : public physx::PxQueryFilterCallback
{
    virtual physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData0, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags)
    {
        const physx::PxFilterData& filterData1 = shape->getSimulationFilterData();

        if ((filterData0.word1 & filterData1.word2) == 0 &&
            (filterData1.word1 & filterData0.word2) == 0)
        {
            return physx::PxQueryHitType::eNONE;
        }

        return physx::PxQueryHitType::eBLOCK;
    }

    virtual physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit)
    {
        // Post filter should be turned off
        DVASSERT(false);
        return physx::PxQueryHitType::eNONE;
    }
};

PhysicsSystem::SimulationEventCallback::SimulationEventCallback(DAVA::CollisionSingleComponent* targetCollisionSingleComponent)
    : targetCollisionSingleComponent(targetCollisionSingleComponent)
{
    DVASSERT(targetCollisionSingleComponent != nullptr);
}

void PhysicsSystem::SimulationEventCallback::onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onWake(physx::PxActor**, physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onSleep(physx::PxActor**, physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
    for (physx::PxU32 i = 0; i < count; ++i)
    {
        physx::PxTriggerPair& pair = pairs[i];

        if (pair.flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER |
                          physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
        {
            // ignore pairs when shapes have been deleted
            continue;
        }

        CollisionShapeComponent* triggerCollisionComponent = CollisionShapeComponent::GetComponent(pair.triggerShape);
        DVASSERT(triggerCollisionComponent);

        CollisionShapeComponent* otherCollisionComponent = CollisionShapeComponent::GetComponent(pair.otherShape);
        DVASSERT(otherCollisionComponent);

        Entity* triggerEntity = triggerCollisionComponent->GetEntity();
        DVASSERT(triggerEntity);

        Entity* otherEntity = otherCollisionComponent->GetEntity();
        DVASSERT(otherEntity);

        // Register trigger event
        TriggerInfo triggerInfo;
        triggerInfo.trigger = triggerEntity;
        triggerInfo.other = otherEntity;

        targetCollisionSingleComponent->activeTriggers.push_back(triggerInfo);
    }
}

void PhysicsSystem::SimulationEventCallback::onAdvance(const physx::PxRigidBody* const*, const physx::PxTransform*, const physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    // Buffer for extracting physx contact points
    static const size_t MAX_CONTACT_POINTS_COUNT = 10;
    static physx::PxContactPairPoint physxContactPoints[MAX_CONTACT_POINTS_COUNT];

    for (physx::PxU32 i = 0; i < nbPairs; ++i)
    {
        const physx::PxContactPair& pair = pairs[i];

        if (pair.contactCount > 0)
        {
            if ((pair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_0) ||
                (pair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_1))
            {
                // Either first or second shape has been removed from the scene
                // Do not report such contacts
                continue;
            }

            // Extract physx points
            const physx::PxU32 contactPointsCount = pair.extractContacts(&physxContactPoints[0],
                                                                         MAX_CONTACT_POINTS_COUNT);
            DVASSERT(contactPointsCount > 0);

            Vector<CollisionPoint> davaContactPoints(contactPointsCount);

            // Convert each contact point from physx structure to engine structure
            for (size_t j = 0; j < contactPointsCount; ++j)
            {
                CollisionPoint& davaPoint = davaContactPoints[j];
                physx::PxContactPairPoint& physxPoint = physxContactPoints[j];

                davaPoint.position = PhysicsMath::PxVec3ToVector3(physxPoint.position);
                davaPoint.normal = PhysicsMath::PxVec3ToVector3(physxPoint.normal);
                davaPoint.impulse = PhysicsMath::PxVec3ToVector3(physxPoint.impulse);
            }

            Component* firstCollisionComponent = static_cast<Component*>(pair.shapes[0]->userData);
            DVASSERT(firstCollisionComponent != nullptr);

            Component* secondCollisionComponent = static_cast<Component*>(pair.shapes[1]->userData);
            DVASSERT(secondCollisionComponent != nullptr);

            Entity* firstEntity = firstCollisionComponent->GetEntity();
            DVASSERT(firstEntity != nullptr);

            Entity* secondEntity = secondCollisionComponent->GetEntity();
            DVASSERT(secondEntity != nullptr);

            // Register collision
            CollisionInfo collisionInfo;
            collisionInfo.first = firstEntity;
            collisionInfo.second = secondEntity;
            collisionInfo.points = std::move(davaContactPoints);
            targetCollisionSingleComponent->collisions.push_back(collisionInfo);
        }
    }
}

PhysicsSystem::PhysicsSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
    Engine* engine = Engine::Instance();

    uint32 threadCount = 2;
    Vector3 gravity(0.0, 0.0, -9.81f);

    simulationBlockSize = PhysicsSystemDetail::DEFAULT_SIMULATION_BLOCK_SIZE;
    simulationEventCallback = SimulationEventCallback(scene->GetSingletonComponent<CollisionSingleComponent>());

    if (engine != nullptr)
    {
        const KeyedArchive* options = engine->GetOptions();

        simulationBlockSize = options->GetUInt32("physics.simulationBlockSize", PhysicsSystemDetail::DEFAULT_SIMULATION_BLOCK_SIZE);
        DVASSERT((simulationBlockSize % (16 * 1024)) == 0); // simulationBlockSize must be 16K multiplier

        gravity = options->GetVector3("physics.gravity", gravity);
        threadCount = options->GetUInt32("physics.threadCount", threadCount);
    }

    const EngineContext* ctx = GetEngineContext();
    PhysicsModule* physics = ctx->moduleManager->GetModule<PhysicsModule>();
    simulationBlock = physics->Allocate(simulationBlockSize, "SimulationBlock", __FILE__, __LINE__);

    PhysicsSceneConfig sceneConfig;
    sceneConfig.gravity = gravity;
    sceneConfig.threadCount = threadCount;

    geometryCache = new PhysicsGeometryCache();

    physicsScene = physics->CreateScene(sceneConfig, FilterShader, &simulationEventCallback);

    vehiclesSubsystem = new PhysicsVehiclesSubsystem(scene, physicsScene);
    controllerManager = PxCreateControllerManager(*physicsScene);

    // Component groups for all physics components
    staticBodies = scene->AquireComponentGroup<StaticBodyComponent, StaticBodyComponent>();
    dynamicBodies = scene->AquireComponentGroup<DynamicBodyComponent, DynamicBodyComponent>();
    shapes = scene->AquireComponentGroupWithMatcher<AnyOfEntityMatcher,
                                                    CollisionShapeComponent,
                                                    BoxShapeComponent, SphereShapeComponent, CapsuleShapeComponent, PlaneShapeComponent, MeshShapeComponent, ConvexHullShapeComponent, HeightFieldShapeComponent>();
    boxCCTs = scene->AquireComponentGroup<BoxCharacterControllerComponent, BoxCharacterControllerComponent>();
    capsuleCCTs = scene->AquireComponentGroup<CapsuleCharacterControllerComponent, CapsuleCharacterControllerComponent>();

    staticBodies->onComponentRemoved->Connect(this, [this](StaticBodyComponent* body) { DeinitBodyComponent(body); });
    dynamicBodies->onComponentRemoved->Connect(this, [this](DynamicBodyComponent* body) { DeinitBodyComponent(body); });
    shapes->onComponentRemoved->Connect(this, [this](CollisionShapeComponent* shape) { DeinitShapeComponent(shape); });
    boxCCTs->onComponentRemoved->Connect(this, [this](BoxCharacterControllerComponent* cct) { DeinitCCTComponent(cct); });
    capsuleCCTs->onComponentRemoved->Connect(this, [this](CapsuleCharacterControllerComponent* cct) { DeinitCCTComponent(cct); });

    // Entity groups for render component dependent shapes
    convexHullAndRenderEntities = scene->AquireEntityGroup<RenderComponent, ConvexHullShapeComponent>();
    meshAndRenderEntities = scene->AquireEntityGroup<RenderComponent, MeshShapeComponent>();
    heightFieldAndRenderEntities = scene->AquireEntityGroup<RenderComponent, HeightFieldShapeComponent>();

    convexHullAndRenderEntities->onEntityAdded->Connect(this, &PhysicsSystem::OnRenderedEntityReady);
    convexHullAndRenderEntities->onEntityRemoved->Connect(this, &PhysicsSystem::OnRenderedEntityNotReady);
    meshAndRenderEntities->onEntityAdded->Connect(this, &PhysicsSystem::OnRenderedEntityReady);
    meshAndRenderEntities->onEntityRemoved->Connect(this, &PhysicsSystem::OnRenderedEntityNotReady);
    heightFieldAndRenderEntities->onEntityAdded->Connect(this, &PhysicsSystem::OnRenderedEntityReady);
    heightFieldAndRenderEntities->onEntityRemoved->Connect(this, &PhysicsSystem::OnRenderedEntityNotReady);

    // Component groups for newly created shapes
    staticBodiesPendingAdd.reset(new ComponentGroupOnAdd<StaticBodyComponent>(staticBodies));
    dynamicBodiesPendingAdd.reset(new ComponentGroupOnAdd<DynamicBodyComponent>(dynamicBodies));
    shapesPendingAdd.reset(new ComponentGroupOnAdd<CollisionShapeComponent>(shapes));
    boxCCTsPendingAdd.reset(new ComponentGroupOnAdd<BoxCharacterControllerComponent>(GetScene()->AquireComponentGroup<BoxCharacterControllerComponent, BoxCharacterControllerComponent>()));
    capsuleCCTsPendingAdd.reset(new ComponentGroupOnAdd<CapsuleCharacterControllerComponent>(GetScene()->AquireComponentGroup<CapsuleCharacterControllerComponent, CapsuleCharacterControllerComponent>()));

    SetDebugDrawEnabled(false);
}

PhysicsSystem::~PhysicsSystem()
{
    if (isSimulationRunning)
    {
        FetchResults(true);
    }

    SafeDelete(vehiclesSubsystem);

    DVASSERT(simulationBlock != nullptr);
    SafeDelete(geometryCache);

    controllerManager->release();

    const EngineContext* ctx = GetEngineContext();
    PhysicsModule* physics = ctx->moduleManager->GetModule<PhysicsModule>();
    physics->Deallocate(simulationBlock);
    simulationBlock = nullptr;
    physicsScene->release();

    staticBodies->onComponentRemoved->Disconnect(this);
    dynamicBodies->onComponentRemoved->Disconnect(this);
    shapes->onComponentRemoved->Disconnect(this);
    boxCCTs->onComponentRemoved->Disconnect(this);
    capsuleCCTs->onComponentRemoved->Disconnect(this);

    convexHullAndRenderEntities->onEntityAdded->Disconnect(this);
    convexHullAndRenderEntities->onEntityRemoved->Disconnect(this);
    meshAndRenderEntities->onEntityAdded->Disconnect(this);
    meshAndRenderEntities->onEntityRemoved->Disconnect(this);
    heightFieldAndRenderEntities->onEntityAdded->Disconnect(this);
    heightFieldAndRenderEntities->onEntityRemoved->Disconnect(this);
}

void PhysicsSystem::UnregisterEntity(Entity* entity)
{
    GetScene()->GetSingletonComponent<CollisionSingleComponent>()->RemoveCollisionsWithEntity(entity);

    physicsEntities.erase(entity);
}

void PhysicsSystem::PrepareForRemove()
{
    physicsComponensUpdatePending.clear();
    collisionComponentsUpdatePending.clear();
    characterControllerComponentsUpdatePending.clear();
    teleportedCcts.clear();
    readyRenderedEntities.clear();
    physicsEntities.clear();

    for (CollisionShapeComponent* component : shapes->components)
    {
        DeinitShapeComponent(component);
    }

    ExecuteForEachBody(MakeFunction(this, &PhysicsSystem::DeinitBodyComponent));
    ExecuteForEachCCT(MakeFunction(this, &PhysicsSystem::DeinitCCTComponent));
}

void PhysicsSystem::ProcessFixedFetch(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("PhysicsSystem::ProcessFixedFetch");

    CollisionSingleComponent* csc = GetScene()->GetSingletonComponent<CollisionSingleComponent>();
    csc->Clear();
    SyncTransformToPhysx();
    MoveCharacterControllers(timeElapsed);
    FetchResults(true);
}

void PhysicsSystem::ProcessFixedSimulate(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("PhysicsSystem::ProcessFixedSimulate");

    InitNewObjects();
    UpdateComponents();

    if (isSimulationEnabled)
    {
        SyncTransformToPhysx();

        DrawDebugInfo();

        ApplyForces();

        for (auto cctInfo : teleportedCcts)
        {
            UpdateCCTFilterData(cctInfo.first, 0, 0);
        }

        vehiclesSubsystem->ProcessFixed(timeElapsed);
        physicsScene->simulate(timeElapsed, nullptr, simulationBlock, simulationBlockSize);

        for (auto cctInfo : teleportedCcts)
        {
            UpdateCCTFilterData(cctInfo.first, cctInfo.second.word1, cctInfo.second.word2);
        }

        teleportedCcts.clear();

        isSimulationRunning = true;
    }
    else
    {
        SyncTransformToPhysx();
    }
}

void PhysicsSystem::SetSimulationEnabled(bool isEnabled)
{
    if (isSimulationEnabled != isEnabled)
    {
        if (isSimulationRunning == true)
        {
            DVASSERT(isSimulationEnabled == true);
            bool success = FetchResults(true);
            DVASSERT(success == true);
        }

        isSimulationEnabled = isEnabled;

        vehiclesSubsystem->OnSimulationEnabled(isSimulationEnabled);
    }
}

bool PhysicsSystem::IsSimulationEnabled() const
{
    return isSimulationEnabled;
}

void PhysicsSystem::SetDebugDrawEnabled(bool drawDebugInfo_)
{
    drawDebugInfo = drawDebugInfo_;
    physx::PxReal enabled = drawDebugInfo == true ? 1.0f : 0.0f;
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_LIN_VELOCITY, enabled);
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_ANG_VELOCITY, enabled);
}

bool PhysicsSystem::IsDebugDrawEnabled() const
{
    return drawDebugInfo;
}

bool PhysicsSystem::FetchResults(bool waitForFetchFinish)
{
    bool isFetched = false;
    if (isSimulationRunning)
    {
        isFetched = physicsScene->fetchResults(waitForFetchFinish);
        if (isFetched == true)
        {
            isSimulationRunning = false;
            physx::PxU32 actorsCount = 0;
            physx::PxActor** actors = physicsScene->getActiveActors(actorsCount);

            // Reset active flag, will be set according to active actors below
            for (DynamicBodyComponent* body : dynamicBodies->components)
            {
                body->isActive = false;
            }

            for (physx::PxU32 i = 0; i < actorsCount; ++i)
            {
                physx::PxActor* actor = actors[i];
                Component* baseComponent = reinterpret_cast<Component*>(actor->userData);

                // When character controller is created, actor is created by physx implicitly
                // In this case there is no PhysicsComponent attached to this entity, only CharacterControllerComponent. We ignore those
                if (baseComponent->GetType()->Is<DynamicBodyComponent>() || baseComponent->GetType()->Is<StaticBodyComponent>())
                {
                    PhysicsComponent* physicsComponent = PhysicsComponent::GetComponent(actor);
                    DVASSERT(physicsComponent != nullptr);

                    if (physicsComponent->GetType()->Is<DynamicBodyComponent>())
                    {
                        static_cast<DynamicBodyComponent*>(physicsComponent)->isActive = true;
                    }

                    Entity* entity = physicsComponent->GetEntity();

                    physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
                    DVASSERT(rigidActor != nullptr);

                    // Update entity's transform and its shapes down the hierarchy recursively
                    TransformComponent* transform = entity->GetComponent<TransformComponent>();
                    transform->SetLocalTransform(PhysicsMath::PxVec3ToVector3(rigidActor->getGlobalPose().p), PhysicsMath::PxQuatToQuaternion(rigidActor->getGlobalPose().q), physicsComponent->currentScale);

                    Vector<Entity*> children;
                    entity->GetChildEntitiesWithCondition(children, [physicsComponent](Entity* e) { return PhysicsSystemDetail::GetParentPhysicsComponent(e) == physicsComponent; });

                    for (Entity* child : children)
                    {
                        DVASSERT(child != nullptr);

                        Vector<CollisionShapeComponent*> shapes = PhysicsUtils::GetShapeComponents(child);
                        if (shapes.size() > 0)
                        {
                            // Update entity using just first shape for now
                            CollisionShapeComponent* shape = shapes[0];
                            if (shape->GetPxShape() != nullptr)
                            {
                                const physx::PxTransform& shapeLocalPos = shape->GetPxShape()->getLocalPose();
                                TransformComponent* childTransform = child->GetComponent<TransformComponent>();
                                childTransform->SetLocalTransform(PhysicsMath::PxVec3ToVector3(shapeLocalPos.p), PhysicsMath::PxQuatToQuaternion(shapeLocalPos.q), shape->scale);
                            }
                        }
                    }

                    if (physicsComponent->GetType()->Is<DynamicBodyComponent>())
                    {
                        DynamicBodyComponent* dynamicBodyComponent = static_cast<DynamicBodyComponent*>(physicsComponent);

                        physx::PxRigidDynamic* dynamicActor = rigidActor->is<physx::PxRigidDynamic>();

                        if (dynamicBodyComponent->GetIsKinematic() == false)
                        {
                            // Do not use SetLinearVelocity/SetAngularVelocity since it will trigger ScheduleUpdate which we do not need
                            dynamicBodyComponent->linearVelocity = PhysicsMath::PxVec3ToVector3(dynamicActor->getLinearVelocity());
                            dynamicBodyComponent->angularVelocity = PhysicsMath::PxVec3ToVector3(dynamicActor->getAngularVelocity());
                        }
                    }
                }
            }
        }
    }

    return isFetched;
}

void PhysicsSystem::DrawDebugInfo()
{
    DVASSERT(isSimulationRunning == false);
    DVASSERT(isSimulationEnabled == true);
    if (IsDebugDrawEnabled() == false)
    {
        return;
    }

    RenderHelper* renderHelper = GetScene()->GetRenderSystem()->GetDebugDrawer();
    const physx::PxRenderBuffer& rb = physicsScene->getRenderBuffer();
    const physx::PxDebugLine* lines = rb.getLines();
    for (physx::PxU32 i = 0; i < rb.getNbLines(); ++i)
    {
        const physx::PxDebugLine& line = lines[i];
        renderHelper->DrawLine(PhysicsMath::PxVec3ToVector3(line.pos0), PhysicsMath::PxVec3ToVector3(line.pos1),
                               PhysicsMath::PxColorToColor(line.color0));
    }

    const physx::PxDebugTriangle* triangles = rb.getTriangles();
    for (physx::PxU32 i = 0; i < rb.getNbTriangles(); ++i)
    {
        const physx::PxDebugTriangle& triangle = triangles[i];
        Polygon3 polygon;
        polygon.AddPoint(PhysicsMath::PxVec3ToVector3(triangle.pos0));
        polygon.AddPoint(PhysicsMath::PxVec3ToVector3(triangle.pos1));
        polygon.AddPoint(PhysicsMath::PxVec3ToVector3(triangle.pos2));
        renderHelper->DrawPolygon(polygon, PhysicsMath::PxColorToColor(triangle.color0), RenderHelper::DRAW_WIRE_DEPTH);
    }

    const physx::PxDebugPoint* points = rb.getPoints();
    for (physx::PxU32 i = 0; i < rb.getNbPoints(); ++i)
    {
        const physx::PxDebugPoint& point = points[i];
        renderHelper->DrawIcosahedron(PhysicsMath::PxVec3ToVector3(point.pos), 5.0f, PhysicsMath::PxColorToColor(point.color), RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void PhysicsSystem::InitNewObjects()
{
    ExecuteForEachPendingBody([this](PhysicsComponent* body) { InitBodyComponent(body); });
    for (CollisionShapeComponent* component : shapesPendingAdd->components)
    {
        InitShapeComponent(component);
    }
    shapesPendingAdd->components.clear();
    ExecuteForEachPendingCCT([this](CharacterControllerComponent* cct) { InitCCTComponent(cct); });

    for (Entity* e : readyRenderedEntities)
    {
        auto processRenderDependentShapes = [this](Entity* entity, const Type* shapeType)
        {
            uint32 numShapes = entity->GetComponentCount(shapeType);
            for (uint32 i = 0; i < numShapes; ++i)
            {
                CollisionShapeComponent* shapeComponent = static_cast<CollisionShapeComponent*>(entity->GetComponent(shapeType, i));

                if (shapeComponent->GetPxShape() == nullptr)
                {
                    InitShapeComponent(shapeComponent);
                    DVASSERT(shapeComponent->shape != nullptr);
                }
            }
        };

        processRenderDependentShapes(e, Type::Instance<ConvexHullShapeComponent>());
        processRenderDependentShapes(e, Type::Instance<MeshShapeComponent>());
        processRenderDependentShapes(e, Type::Instance<HeightFieldShapeComponent>());
    }
    readyRenderedEntities.clear();
}

void PhysicsSystem::AttachShape(PhysicsComponent* bodyComponent, CollisionShapeComponent* shapeComponent, const Vector3& scale)
{
    physx::PxActor* actor = bodyComponent->GetPxActor();
    DVASSERT(actor);
    physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
    DVASSERT(rigidActor);

    shapeComponent->scale = scale;

    physx::PxShape* shape = shapeComponent->GetPxShape();
    if (shape != nullptr)
    {
        rigidActor->attachShape(*shape);
        ScheduleUpdate(shapeComponent);
        ScheduleUpdate(bodyComponent);
    }
}

void PhysicsSystem::AttachShapesRecursively(Entity* entity, PhysicsComponent* bodyComponent, const Vector3& scale)
{
    for (const Type* type : GetEngineContext()->moduleManager->GetModule<PhysicsModule>()->GetShapeComponentTypes())
    {
        for (uint32 i = 0; i < entity->GetComponentCount(type); ++i)
        {
            AttachShape(bodyComponent, static_cast<CollisionShapeComponent*>(entity->GetComponent(type, i)), scale);
        }
    }

    const int32 childrenCount = entity->GetChildrenCount();
    for (int32 i = 0; i < childrenCount; ++i)
    {
        Entity* child = entity->GetChild(i);
        if (child->GetComponent<DynamicBodyComponent>() || child->GetComponent<StaticBodyComponent>())
        {
            continue;
        }

        AttachShapesRecursively(child, bodyComponent, scale);
    }
}

physx::PxShape* PhysicsSystem::CreateShape(CollisionShapeComponent* component, PhysicsModule* physics)
{
    using namespace PhysicsSystemDetail;
    physx::PxShape* shape = nullptr;

    const Type* componentType = component->GetType();

    if (componentType->Is<BoxShapeComponent>())
    {
        BoxShapeComponent* boxShape = static_cast<BoxShapeComponent*>(component);
        shape = physics->CreateBoxShape(boxShape->GetHalfSize(), component->GetMaterialName());
    }

    else if (componentType->Is<CapsuleShapeComponent>())
    {
        CapsuleShapeComponent* capsuleShape = static_cast<CapsuleShapeComponent*>(component);
        shape = physics->CreateCapsuleShape(capsuleShape->GetRadius(), capsuleShape->GetHalfHeight(), component->GetMaterialName());
    }

    else if (componentType->Is<SphereShapeComponent>())
    {
        SphereShapeComponent* sphereShape = static_cast<SphereShapeComponent*>(component);
        shape = physics->CreateSphereShape(sphereShape->GetRadius(), component->GetMaterialName());
    }

    else if (componentType->Is<PlaneShapeComponent>())
    {
        shape = physics->CreatePlaneShape(component->GetMaterialName());
    }

    else if (componentType->Is<ConvexHullShapeComponent>())
    {
        Vector<PolygonGroup*> groups;
        Entity* entity = component->GetEntity();
        Vector3 scale = AccumulateMeshInfo(entity, groups);
        if (groups.empty() == false)
        {
            shape = physics->CreateConvexHullShape(std::move(groups), scale, component->GetMaterialName(), geometryCache);
        }
    }

    else if (componentType->Is<MeshShapeComponent>())
    {
        Vector<PolygonGroup*> groups;
        Entity* entity = component->GetEntity();
        Vector3 scale = AccumulateMeshInfo(entity, groups);
        if (groups.empty() == false)
        {
            shape = physics->CreateMeshShape(std::move(groups), scale, component->GetMaterialName(), geometryCache);
        }
    }

    else if (componentType->Is<HeightFieldShapeComponent>())
    {
        Entity* entity = component->GetEntity();
        Landscape* landscape = GetLandscape(entity);
        if (landscape != nullptr)
        {
            DVASSERT(landscape->GetHeightmap() != nullptr);

            Matrix4 localPose;
            shape = physics->CreateHeightField(landscape, component->GetMaterialName(), localPose);
            component->SetLocalPose(localPose);
        }
    }

    else
    {
        DVASSERT(false);
    }

    if (shape != nullptr)
    {
        component->SetPxShape(shape);

        if (component->GetType()->Is<HeightFieldShapeComponent>() || component->GetType()->Is<PlaneShapeComponent>())
        {
            vehiclesSubsystem->SetupDrivableSurface(component);
        }
    }

    return shape;
}

void PhysicsSystem::SyncTransformToPhysx()
{
    TransformSingleComponent* transformSingle = GetScene()->GetSingletonComponent<TransformSingleComponent>();
    for (Entity* entity : transformSingle->localTransformChanged)
    {
        // Check for perfomance reason: if this entity is not particiapting in physics simulation,
        // there is no need to handle it
        if (physicsEntities.find(entity) != physicsEntities.end())
        {
            SyncEntityTransformToPhysx(entity);
        }
    }
}

void PhysicsSystem::SyncEntityTransformToPhysx(Entity* entity)
{
    auto updateBodyComponent = [](Entity* e, PhysicsComponent* component)
    {
        if (component != nullptr)
        {
            physx::PxActor* actor = component->GetPxActor();
            if (actor != nullptr)
            {
                physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
                DVASSERT(rigidActor != nullptr);

                TransformComponent* transform = e->GetComponent<TransformComponent>();
                PhysicsSystemDetail::UpdateActorGlobalPose(rigidActor, transform->GetPosition(), transform->GetRotation());
                component->currentScale = transform->GetScale();

                physx::PxU32 shapesCount = rigidActor->getNbShapes();
                for (physx::PxU32 i = 0; i < shapesCount; ++i)
                {
                    physx::PxShape* shape = nullptr;
                    rigidActor->getShapes(&shape, 1, i);

                    // Update local pose for nested shapes
                    CollisionShapeComponent* shapeComponent = CollisionShapeComponent::GetComponent(shape);
                    DVASSERT(shapeComponent->GetEntity() != nullptr);
                    if (shapeComponent->GetEntity() != e)
                    {
                        TransformComponent* childTransform = shapeComponent->GetEntity()->GetComponent<TransformComponent>();
                        PhysicsSystemDetail::UpdateShapeLocalPose(shape, childTransform->GetPosition(), childTransform->GetRotation());
                    }

                    // Update geometry scale
                    PhysicsSystemDetail::UpdateShapeGeometryScale(shape, transform->GetScale());
                }
            }
        }
    };

    const bool isRootEntity = entity->GetParent() == entity->GetScene();

    // Actors and CCTs can only be root entities since their positions are represented by global positions
    // If passed entity is nested, then check for shapes and update their local positions
    if (isRootEntity)
    {
        // Update actor
        PhysicsComponent* bodyComponent = PhysicsUtils::GetBodyComponent(entity);
        if (bodyComponent != nullptr)
        {
            updateBodyComponent(entity, bodyComponent);
        }

        // Update CCT if there is no body
        if (bodyComponent == nullptr)
        {
            CharacterControllerComponent* controller = PhysicsUtils::GetCharacterControllerComponent(entity);
            if (controller != nullptr)
            {
                TransformComponent* transform = entity->GetComponent<TransformComponent>();
                physx::PxController* pxController = controller->controller;
                if (controller != nullptr)
                {
                    Vector3 currentPosition = PhysicsMath::PxExtendedVec3ToVector3(pxController->getFootPosition());
                    Vector3 newPosition = transform->GetPosition();

                    if (!FLOAT_EQUAL(currentPosition.x, newPosition.x) ||
                        !FLOAT_EQUAL(currentPosition.y, newPosition.y) ||
                        !FLOAT_EQUAL(currentPosition.z, newPosition.z))
                    {
                        pxController->setFootPosition(PhysicsMath::Vector3ToPxExtendedVec3(newPosition));

                        physx::PxShape* cctShape = nullptr;
                        pxController->getActor()->getShapes(&cctShape, 1, 0);
                        DVASSERT(cctShape != nullptr);
                        teleportedCcts.insert({ controller, cctShape->getSimulationFilterData() });
                    }
                }
            }
        }
    }
    else
    {
        // Update shapes
        Vector<CollisionShapeComponent*> shapeComponents = PhysicsUtils::GetShapeComponents(entity);
        if (shapeComponents.size() > 0)
        {
            CollisionShapeComponent* shapeComponent = shapeComponents[0];
            physx::PxShape* shape = shapeComponent->GetPxShape();
            if (shape != nullptr)
            {
                TransformComponent* transform = entity->GetComponent<TransformComponent>();
                PhysicsSystemDetail::UpdateShapeLocalPose(shapeComponent->GetPxShape(), transform->GetPosition(), transform->GetRotation());
            }
        }
    }
}

void PhysicsSystem::ReleaseShape(CollisionShapeComponent* component)
{
    physx::PxShape* shape = component->GetPxShape();
    if (shape == nullptr)
    {
        return;
    }
    DVASSERT(shape->isExclusive() == true);

    physx::PxActor* actor = shape->getActor();
    if (actor != nullptr)
    {
        actor->is<physx::PxRigidActor>()->detachShape(*shape);
    }

    component->ReleasePxShape();
}

void PhysicsSystem::ScheduleUpdate(PhysicsComponent* component)
{
    physicsComponensUpdatePending.insert(component);
}

void PhysicsSystem::ScheduleUpdate(CollisionShapeComponent* component)
{
    collisionComponentsUpdatePending.insert(component);
}

void PhysicsSystem::ScheduleUpdate(CharacterControllerComponent* component)
{
    characterControllerComponentsUpdatePending.insert(component);
}

bool PhysicsSystem::Raycast(const Vector3& origin, const Vector3& direction, float32 distance, physx::PxRaycastCallback& callback, const physx::PxQueryFilterData& filterData, physx::PxQueryFilterCallback* filterCall)
{
    using namespace physx;

    return physicsScene->raycast(PhysicsMath::Vector3ToPxVec3(origin), PhysicsMath::Vector3ToPxVec3(Normalize(direction)),
                                 static_cast<PxReal>(distance), callback, PxHitFlags(PxHitFlag::eDEFAULT), filterData, filterCall);
}

PhysicsVehiclesSubsystem* PhysicsSystem::GetVehiclesSystem()
{
    return vehiclesSubsystem;
}

void PhysicsSystem::UpdateComponents()
{
    PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    for (CollisionShapeComponent* shapeComponent : collisionComponentsUpdatePending)
    {
        shapeComponent->UpdateLocalProperties();
        physx::PxShape* shape = shapeComponent->GetPxShape();
        DVASSERT(shape != nullptr);
        physx::PxMaterial* material = module->GetMaterial(shapeComponent->GetMaterialName());
        shape->setMaterials(&material, 1);
        physx::PxActor* actor = shape->getActor();
        if (actor != nullptr)
        {
            PhysicsComponent* bodyComponent = PhysicsComponent::GetComponent(actor);
            physicsComponensUpdatePending.insert(bodyComponent);
        }
    }

    for (PhysicsComponent* bodyComponent : physicsComponensUpdatePending)
    {
        bodyComponent->UpdateLocalProperties();

        // Recalculate mass
        // Ignore vehicles, VehiclesSubsystem is responsible for setting correct values

        Entity* entity = bodyComponent->GetEntity();
        if (entity->GetComponent<VehicleCarComponent>() == nullptr &&
            entity->GetComponent<VehicleTankComponent>() == nullptr)
        {
            physx::PxRigidDynamic* dynamicActor = bodyComponent->GetPxActor()->is<physx::PxRigidDynamic>();
            if (dynamicActor != nullptr)
            {
                physx::PxU32 shapesCount = dynamicActor->getNbShapes();
                if (shapesCount > 0)
                {
                    Vector<physx::PxShape*> shapes(shapesCount, nullptr);
                    physx::PxU32 extractedShapesCount = dynamicActor->getShapes(shapes.data(), shapesCount);
                    DVASSERT(shapesCount == extractedShapesCount);

                    Vector<physx::PxReal> masses;
                    masses.reserve(shapesCount);

                    for (physx::PxShape* shape : shapes)
                    {
                        CollisionShapeComponent* shapeComponent = CollisionShapeComponent::GetComponent(shape);
                        masses.push_back(shapeComponent->GetMass());
                    }

                    physx::PxRigidBodyExt::setMassAndUpdateInertia(*dynamicActor, masses.data(), static_cast<physx::PxU32>(masses.size()));
                }
            }
        }

        if (bodyComponent->GetType()->Is<DynamicBodyComponent>())
        {
            DynamicBodyComponent* dynamicBody = static_cast<DynamicBodyComponent*>(bodyComponent);
            bool isCCDEnabled = dynamicBody->IsCCDEnabled();

            physx::PxRigidDynamic* actor = dynamicBody->GetPxActor()->is<physx::PxRigidDynamic>();
            DVASSERT(actor != nullptr);

            physx::PxU32 shapesCount = actor->getNbShapes();
            for (physx::PxU32 shapeIndex = 0; shapeIndex < shapesCount; ++shapeIndex)
            {
                physx::PxShape* shape = nullptr;
                actor->getShapes(&shape, 1, shapeIndex);
                CollisionShapeComponent::SetCCDEnabled(shape, isCCDEnabled);
            }
        }
    }

    for (CharacterControllerComponent* controllerComponent : characterControllerComponentsUpdatePending)
    {
        physx::PxController* controller = controllerComponent->controller;
        if (controller != nullptr)
        {
            // Update geometry if needed
            if (controllerComponent->geometryChanged)
            {
                if (controllerComponent->GetType()->Is<BoxCharacterControllerComponent>())
                {
                    BoxCharacterControllerComponent* boxComponent = static_cast<BoxCharacterControllerComponent*>(controllerComponent);
                    physx::PxBoxController* boxController = static_cast<physx::PxBoxController*>(controller);

                    boxController->setHalfHeight(boxComponent->GetHalfHeight());
                    boxController->setHalfForwardExtent(boxComponent->GetHalfForwardExtent());
                    boxController->setHalfSideExtent(boxComponent->GetHalfSideExtent());
                }
                else if (controllerComponent->GetType()->Is<CapsuleCharacterControllerComponent>())
                {
                    CapsuleCharacterControllerComponent* capsuleComponent = static_cast<CapsuleCharacterControllerComponent*>(controllerComponent);
                    physx::PxCapsuleController* capsuleController = static_cast<physx::PxCapsuleController*>(controller);

                    capsuleController->setRadius(capsuleComponent->GetRadius());
                    capsuleController->setHeight(capsuleComponent->GetHeight());
                }

                controllerComponent->geometryChanged = false;
            }

            controller->setContactOffset(controllerComponent->GetContactOffset());

            // Teleport if needed
            if (controllerComponent->teleported)
            {
                controller->setFootPosition(PhysicsMath::Vector3ToPxExtendedVec3(controllerComponent->teleportDestination));
                controllerComponent->teleported = false;

                physx::PxShape* cctShape = nullptr;
                controller->getActor()->getShapes(&cctShape, 1, 0);
                DVASSERT(cctShape != nullptr);
                teleportedCcts.insert({ controllerComponent, cctShape->getSimulationFilterData() });
            }

            // Update filter data
            UpdateCCTFilterData(controllerComponent, controllerComponent->GetTypeMask(), controllerComponent->GetTypeMaskToCollideWith());
        }
    }

    collisionComponentsUpdatePending.clear();
    physicsComponensUpdatePending.clear();
    characterControllerComponentsUpdatePending.clear();
}

void PhysicsSystem::MoveCharacterControllers(float32 timeElapsed)
{
    ExecuteForEachCCT([this, timeElapsed](CharacterControllerComponent* controllerComponent)
                      {
                          physx::PxController* controller = controllerComponent->controller;
                          if (controller != nullptr)
                          {
                              // Apply movement

                              physx::PxShape* cctShape = nullptr;
                              controller->getActor()->getShapes(&cctShape, 1, 0);
                              DVASSERT(cctShape != nullptr);

                              CCTQueryFilterCallback filterCallback;
                              physx::PxFilterData filterData = cctShape->getSimulationFilterData();

                              physx::PxControllerFilters filter;
                              filter.mFilterCallback = &filterCallback;
                              filter.mFilterData = &filterData;
                              filter.mFilterFlags = physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;

                              physx::PxControllerCollisionFlags collisionFlags;
                              if (controllerComponent->GetMovementMode() == CharacterControllerComponent::MovementMode::Flying)
                              {
                                  collisionFlags = controller->move(PhysicsMath::Vector3ToPxVec3(controllerComponent->totalDisplacement), 0.0f, timeElapsed, filter);
                              }
                              else
                              {
                                  DVASSERT(controllerComponent->GetMovementMode() == CharacterControllerComponent::MovementMode::Walking);

                                  // Ignore displacement along z axis
                                  Vector3 displacement = controllerComponent->totalDisplacement;
                                  displacement.z = 0.0f;

                                  // Apply gravity
                                  displacement += PhysicsMath::PxVec3ToVector3(physicsScene->getGravity()) * timeElapsed;
                                  collisionFlags = controller->move(PhysicsMath::Vector3ToPxVec3(displacement), 0.0f, timeElapsed, filter);
                              }

                              controllerComponent->grounded = (collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
                              controllerComponent->totalDisplacement = Vector3::Zero;

                              // Sync entity's transform

                              Entity* entity = controllerComponent->GetEntity();
                              DVASSERT(entity != nullptr);

                              TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
                              DVASSERT(transformComponent != nullptr);

                              Vector3 newPosition = PhysicsMath::PxExtendedVec3ToVector3(controller->getFootPosition());
                              Vector3 const& oldPosition = transformComponent->GetPosition();
                              if (!FLOAT_EQUAL(newPosition.x, oldPosition.x) || !FLOAT_EQUAL(newPosition.y, oldPosition.y) || !FLOAT_EQUAL(newPosition.z, oldPosition.z))
                              {
                                  transformComponent->SetLocalTransform(newPosition, transformComponent->GetRotation(), transformComponent->GetScale());
                              }
                          }
                      });
}

void PhysicsSystem::UpdateCCTFilterData(CharacterControllerComponent* cctComponent, uint32 typeMask, uint32 typeMaskToCollideWith)
{
    DVASSERT(cctComponent != nullptr);

    physx::PxShape* controllerShape = nullptr;
    cctComponent->controller->getActor()->getShapes(&controllerShape, 1, 0);
    DVASSERT(controllerShape != nullptr);

    physx::PxFilterData simFilterData = controllerShape->getSimulationFilterData();
    simFilterData.word1 = typeMask;
    simFilterData.word2 = typeMaskToCollideWith;
    controllerShape->setSimulationFilterData(simFilterData);

    physx::PxFilterData queryFilterData = controllerShape->getQueryFilterData();
    queryFilterData.word1 = typeMask;
    queryFilterData.word2 = typeMaskToCollideWith;
    controllerShape->setQueryFilterData(queryFilterData);

    cctComponent->controller->invalidateCache();
}

void PhysicsSystem::AddForce(DynamicBodyComponent* component, const Vector3& force, physx::PxForceMode::Enum mode)
{
    PendingForce pendingForce;
    pendingForce.component = component;
    pendingForce.force = force;
    pendingForce.mode = mode;

    forces.push_back(pendingForce);
}

void PhysicsSystem::ApplyForces()
{
    for (const PendingForce& force : forces)
    {
        physx::PxActor* actor = force.component->GetPxActor();
        DVASSERT(actor != nullptr);
        physx::PxRigidBody* rigidBody = actor->is<physx::PxRigidBody>();
        DVASSERT(rigidBody != nullptr);

        rigidBody->addForce(PhysicsMath::Vector3ToPxVec3(force.force), force.mode);
    }

    forces.clear();
}

void PhysicsSystem::InitBodyComponent(PhysicsComponent* bodyComponent)
{
    PhysicsModule* physics = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    DVASSERT(physics != nullptr);

    Entity* entity = bodyComponent->GetEntity();
    DVASSERT(entity != nullptr);

    // Body components are only allowed to be root objects
    DVASSERT(entity->GetParent() == entity->GetScene());

    // Create underlying PxActor
    physx::PxRigidActor* createdActor = nullptr;
    if (bodyComponent->GetType()->Is<StaticBodyComponent>())
    {
        createdActor = physics->CreateStaticActor();
    }
    else
    {
        DVASSERT(bodyComponent->GetType()->Is<DynamicBodyComponent>());
        createdActor = physics->CreateDynamicActor();
    }
    DVASSERT(createdActor != nullptr);

    bodyComponent->SetPxActor(createdActor);

    TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
    DVASSERT(transformComponent != nullptr);

    PhysicsSystemDetail::UpdateActorGlobalPose(createdActor, transformComponent->GetPosition(), transformComponent->GetRotation());
    bodyComponent->currentScale = transformComponent->GetScale();

    AttachShapesRecursively(entity, bodyComponent, transformComponent->GetScale());

    physicsScene->addActor(*createdActor);

    physicsEntities.insert(entity);
}

void PhysicsSystem::InitShapeComponent(CollisionShapeComponent* shapeComponent)
{
    PhysicsModule* physics = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    DVASSERT(physics != nullptr);

    Entity* entity = shapeComponent->GetEntity();
    DVASSERT(entity != nullptr);

    physx::PxShape* shape = CreateShape(shapeComponent, physics);

    // Shape can be equal to null if it's a mesh or convex hull but no render component has been added to the entity yet
    // If it's null, we will return to this component later when render info is available
    if (shape != nullptr)
    {
        TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
        DVASSERT(transformComponent != nullptr);

        PhysicsComponent* physicsComponent = PhysicsSystemDetail::GetParentPhysicsComponent(entity);
        if (physicsComponent != nullptr)
        {
            AttachShape(physicsComponent, shapeComponent, transformComponent->GetScale());

            if (physicsComponent->GetType()->Is<DynamicBodyComponent>())
            {
                physx::PxRigidDynamic* dynamicActor = physicsComponent->GetPxActor()->is<physx::PxRigidDynamic>();
                DVASSERT(dynamicActor != nullptr);

                if (dynamicActor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_SIMULATION) == false &&
                    dynamicActor->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC) == false)
                {
                    dynamicActor->wakeUp();
                }
            }
        }
        shape->release();
        physicsEntities.insert(entity);
    }
}

void PhysicsSystem::InitCCTComponent(CharacterControllerComponent* cctComponent)
{
    Entity* entity = cctComponent->GetEntity();
    DVASSERT(entity != nullptr);

    // Character controllers are only allowed to be root objects
    DVASSERT(entity->GetParent() == entity->GetScene());

    PhysicsModule* physics = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    DVASSERT(physics != nullptr);

    physx::PxController* controller = nullptr;
    if (cctComponent->GetType()->Is<BoxCharacterControllerComponent>())
    {
        BoxCharacterControllerComponent* boxCharacterControllerComponent = static_cast<BoxCharacterControllerComponent*>(cctComponent);

        physx::PxBoxControllerDesc desc;
        desc.position = PhysicsMath::Vector3ToPxExtendedVec3(entity->GetLocalTransform().GetTranslationVector());
        desc.halfHeight = boxCharacterControllerComponent->GetHalfHeight();
        desc.halfForwardExtent = boxCharacterControllerComponent->GetHalfForwardExtent();
        desc.halfSideExtent = boxCharacterControllerComponent->GetHalfSideExtent();
        desc.upDirection = PhysicsMath::Vector3ToPxVec3(Vector3::UnitZ);
        desc.contactOffset = boxCharacterControllerComponent->GetContactOffset();
        desc.scaleCoeff = boxCharacterControllerComponent->GetScaleCoeff();
        desc.material = physics->GetMaterial(FastName());
        DVASSERT(desc.isValid());

        controller = controllerManager->createController(desc);
    }
    else if (cctComponent->GetType()->Is<CapsuleCharacterControllerComponent>())
    {
        CapsuleCharacterControllerComponent* capsuleCharacterControllerComponent = static_cast<CapsuleCharacterControllerComponent*>(cctComponent);

        physx::PxCapsuleControllerDesc desc;
        desc.position = PhysicsMath::Vector3ToPxExtendedVec3(entity->GetLocalTransform().GetTranslationVector());
        desc.radius = capsuleCharacterControllerComponent->GetRadius();
        desc.height = capsuleCharacterControllerComponent->GetHeight();
        desc.contactOffset = capsuleCharacterControllerComponent->GetContactOffset();
        desc.scaleCoeff = capsuleCharacterControllerComponent->GetScaleCoeff();
        desc.material = physics->GetMaterial(FastName());
        desc.upDirection = PhysicsMath::Vector3ToPxVec3(Vector3::UnitZ);
        DVASSERT(desc.isValid());

        controller = controllerManager->createController(desc);

        static_cast<physx::PxCapsuleController*>(controller)->setClimbingMode(physx::PxCapsuleClimbingMode::eCONSTRAINED);
    }

    DVASSERT(controller != nullptr);

    // CCT actor references dava component
    controller->getActor()->userData = static_cast<void*>(cctComponent);

    // CCT actor shapes also reference dava component
    physx::PxShape* cctShape = nullptr;
    controller->getActor()->getShapes(&cctShape, 1, 0);
    DVASSERT(cctShape != nullptr);
    cctShape->userData = static_cast<void*>(cctComponent);

    controller->setStepOffset(0.02f);

    cctComponent->controller = controller;

    UpdateCCTFilterData(cctComponent, cctComponent->GetTypeMask(), cctComponent->GetTypeMaskToCollideWith());

    physicsEntities.insert(entity);
}

void PhysicsSystem::DeinitBodyComponent(PhysicsComponent* bodyComponent)
{
    physicsComponensUpdatePending.erase(bodyComponent);

    physx::PxRigidActor* actor = bodyComponent->GetPxActor();
    if (actor != nullptr)
    {
        physx::PxU32 shapesCount = actor->getNbShapes();
        Vector<physx::PxShape*> shapes(shapesCount, nullptr);
        actor->getShapes(shapes.data(), shapesCount);

        for (physx::PxShape* shape : shapes)
        {
            DVASSERT(shape != nullptr);
            actor->detachShape(*shape);
        }

        physicsScene->removeActor(*actor);
        bodyComponent->ReleasePxActor();
    }

    if (bodyComponent->GetType()->Is<DynamicBodyComponent>())
    {
        size_t index = 0;
        while (index < forces.size())
        {
            PendingForce& force = forces[index];
            if (force.component == bodyComponent)
            {
                RemoveExchangingWithLast(forces, index);
            }
            else
            {
                ++index;
            }
        }
    }

    Entity* entity = bodyComponent->GetEntity();
    if (!PhysicsSystemDetail::IsPhysicsEntity(*entity, *bodyComponent))
    {
        physicsEntities.erase(entity);
    }
}

void PhysicsSystem::DeinitShapeComponent(CollisionShapeComponent* shapeComponent)
{
    DVASSERT(shapeComponent != nullptr);

    Entity* entity = shapeComponent->GetEntity();
    DVASSERT(entity != nullptr);

    collisionComponentsUpdatePending.erase(shapeComponent);

    ReleaseShape(shapeComponent);

    if (!PhysicsSystemDetail::IsPhysicsEntity(*entity, *shapeComponent))
    {
        physicsEntities.erase(entity);
    }
}

void PhysicsSystem::DeinitCCTComponent(CharacterControllerComponent* cctComponent)
{
    if (cctComponent->controller != nullptr)
    {
        cctComponent->controller->release();
    }

    Entity* entity = cctComponent->GetEntity();
    if (!PhysicsSystemDetail::IsPhysicsEntity(*entity, *cctComponent))
    {
        physicsEntities.erase(entity);
    }
}

void PhysicsSystem::OnRenderedEntityReady(Entity* entity)
{
    readyRenderedEntities.insert(entity);
}

void PhysicsSystem::OnRenderedEntityNotReady(Entity* entity)
{
    readyRenderedEntities.erase(entity);

    auto processRenderDependentShapes = [this, entity](const Type* componentType)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(componentType); ++i)
        {
            CollisionShapeComponent* component = static_cast<CollisionShapeComponent*>(entity->GetComponent(componentType, i));
            ReleaseShape(component);
        }
    };

    processRenderDependentShapes(Type::Instance<ConvexHullShapeComponent>());
    processRenderDependentShapes(Type::Instance<MeshShapeComponent>());
    processRenderDependentShapes(Type::Instance<HeightFieldShapeComponent>());
}

void PhysicsSystem::ExecuteForEachBody(Function<void(PhysicsComponent*)> func)
{
    for (StaticBodyComponent* component : staticBodies->components)
    {
        func(component);
    }

    for (DynamicBodyComponent* component : dynamicBodies->components)
    {
        func(component);
    }
}

void PhysicsSystem::ExecuteForEachPendingBody(Function<void(PhysicsComponent*)> func)
{
    for (StaticBodyComponent* component : staticBodiesPendingAdd->components)
    {
        func(component);
    }

    for (DynamicBodyComponent* component : dynamicBodiesPendingAdd->components)
    {
        func(component);
    }

    staticBodiesPendingAdd->components.clear();
    dynamicBodiesPendingAdd->components.clear();
}

void PhysicsSystem::ExecuteForEachCCT(Function<void(CharacterControllerComponent*)> func)
{
    for (BoxCharacterControllerComponent* component : boxCCTs->components)
    {
        func(component);
    }

    for (CapsuleCharacterControllerComponent* component : capsuleCCTs->components)
    {
        func(component);
    }
}

void PhysicsSystem::ExecuteForEachPendingCCT(Function<void(CharacterControllerComponent*)> func)
{
    for (BoxCharacterControllerComponent* component : boxCCTsPendingAdd->components)
    {
        func(component);
    }

    for (CapsuleCharacterControllerComponent* component : capsuleCCTsPendingAdd->components)
    {
        func(component);
    }

    boxCCTsPendingAdd->components.clear();
    capsuleCCTsPendingAdd->components.clear();
}

} // namespace DAVA
