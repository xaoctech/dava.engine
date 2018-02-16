#pragma once

#include "Physics/DynamicBodyComponent.h"
#include "Physics/StaticBodyComponent.h"

#include <Entity/SceneSystem.h>
#include <Math/Vector.h>
#include <Base/BaseTypes.h>
#include <Base/UnordererSet.h>
#include <Scene3D/ComponentGroup.h>

#include <physx/PxQueryReport.h>
#include <physx/PxSimulationEventCallback.h>
#include <physx/PxForceMode.h>
#include <physx/PxQueryFiltering.h>

namespace physx
{
class PxScene;
class PxRigidActor;
class PxShape;
class PxControllerManager;
}

namespace DAVA
{
class Scene;
class CollisionSingleComponent;
class PhysicsModule;
class PhysicsComponent;
class DynamicBodyComponent;
class CollisionShapeComponent;
class BoxShapeComponent;
class SphereShapeComponent;
class CapsuleShapeComponent;
class PlaneShapeComponent;
class MeshShapeComponent;
class ConvexHullShapeComponent;
class HeightFieldShapeComponent;
class PhysicsGeometryCache;
class PhysicsVehiclesSubsystem;
class CharacterControllerComponent;
class BoxCharacterControllerComponent;
class CapsuleCharacterControllerComponent;

class PhysicsSystem final : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(PhysicsSystem, SceneSystem);

    PhysicsSystem(Scene* scene);
    ~PhysicsSystem() override;

    void UnregisterEntity(Entity* entity) override;

    void ProcessFixedFetch(float32 timeElapsed);
    void ProcessFixedSimulate(float32 timeElapsed);
    void PrepareForRemove() override;

    void SetSimulationEnabled(bool isEnabled);
    bool IsSimulationEnabled() const;

    void SetDebugDrawEnabled(bool drawDebugInfo);
    bool IsDebugDrawEnabled() const;

    void ScheduleUpdate(PhysicsComponent* component);
    void ScheduleUpdate(CollisionShapeComponent* component);
    void ScheduleUpdate(CharacterControllerComponent* component);

    bool Raycast(const Vector3& origin, const Vector3& direction, float32 distance, physx::PxRaycastCallback& callback, const physx::PxQueryFilterData& filterData = physx::PxQueryFilterData(), physx::PxQueryFilterCallback* filterCall = nullptr);
    void AddForce(DynamicBodyComponent* component, const Vector3& force, physx::PxForceMode::Enum mode);

    PhysicsVehiclesSubsystem* GetVehiclesSystem();

    void SyncTransformToPhysx();

private:
    bool FetchResults(bool waitForFetchFinish);

    void DrawDebugInfo();

    void InitNewObjects();
    void AttachShapesRecursively(Entity* entity, PhysicsComponent* bodyComponent, const Vector3& scale);
    void AttachShape(PhysicsComponent* bodyComponent, CollisionShapeComponent* shapeComponent, const Vector3& scale);

    void ReleaseShape(CollisionShapeComponent* component);
    physx::PxShape* CreateShape(CollisionShapeComponent* component, PhysicsModule* physics);

    void SyncEntityTransformToPhysx(Entity* entity);
    void UpdateComponents();
    void ApplyForces();

    void MoveCharacterControllers(float32 timeElapsed);

    void UpdateCCTFilterData(CharacterControllerComponent* cctComponent, uint32 typeMask, uint32 typeMaskToCollideWith);

    // Initialization of different component types
    void InitBodyComponent(PhysicsComponent* bodyComponent);
    void InitShapeComponent(CollisionShapeComponent* shapeComponent);
    void InitCCTComponent(CharacterControllerComponent* cctComponent);

    // Uninitialization of different component types
    void DeinitBodyComponent(PhysicsComponent* bodyComponnet);
    void DeinitShapeComponent(CollisionShapeComponent* shapeComponent);
    void DeinitCCTComponent(CharacterControllerComponent* cctComponent);

    void OnRenderedEntityReady(Entity* entity);
    void OnRenderedEntityNotReady(Entity* entity);

    void ExecuteForEachBody(Function<void(PhysicsComponent*)> func);
    void ExecuteForEachPendingBody(Function<void(PhysicsComponent*)> func);
    void ExecuteForEachCCT(Function<void(CharacterControllerComponent*)> func);
    void ExecuteForEachPendingCCT(Function<void(CharacterControllerComponent*)> func);

private:
    class SimulationEventCallback : public physx::PxSimulationEventCallback
    {
    public:
        SimulationEventCallback() = default;
        SimulationEventCallback(CollisionSingleComponent* targetCollisionSingleComponent);
        void onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32) override;
        void onWake(physx::PxActor**, physx::PxU32) override;
        void onSleep(physx::PxActor**, physx::PxU32) override;
        void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
        void onAdvance(const physx::PxRigidBody* const*, const physx::PxTransform*, const physx::PxU32) override;
        void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;

    private:
        CollisionSingleComponent* targetCollisionSingleComponent;
    };

private:
    friend class PhysicsSystemPrivate; // for tests only

    void* simulationBlock = nullptr;
    uint32 simulationBlockSize = 0;

    bool isSimulationEnabled = true;
    bool isSimulationRunning = false;
    physx::PxScene* physicsScene = nullptr;
    physx::PxControllerManager* controllerManager = nullptr;
    PhysicsGeometryCache* geometryCache = nullptr;

    PhysicsVehiclesSubsystem* vehiclesSubsystem = nullptr;

    Set<PhysicsComponent*> physicsComponensUpdatePending;
    Set<CollisionShapeComponent*> collisionComponentsUpdatePending;
    Set<CharacterControllerComponent*> characterControllerComponentsUpdatePending;

    UnorderedMap<CharacterControllerComponent*, physx::PxFilterData> teleportedCcts;

    struct PendingForce
    {
        DynamicBodyComponent* component = nullptr;
        Vector3 force;
        physx::PxForceMode::Enum mode;
    };

    Vector<PendingForce> forces;
    SimulationEventCallback simulationEventCallback;

    bool drawDebugInfo = false;

    // Component groups for different type of physics components
    ComponentGroup<StaticBodyComponent>* staticBodies;
    ComponentGroup<DynamicBodyComponent>* dynamicBodies;
    ComponentGroup<CollisionShapeComponent>* shapes;
    ComponentGroup<BoxCharacterControllerComponent>* boxCCTs;
    ComponentGroup<CapsuleCharacterControllerComponent>* capsuleCCTs;

    // Entity group for entites whose shapes depend on render components
    EntityGroup* convexHullAndRenderEntities;
    EntityGroup* meshAndRenderEntities;
    EntityGroup* heightFieldAndRenderEntities;

    // New components which haven't been handled yet
    std::unique_ptr<ComponentGroupOnAdd<StaticBodyComponent>> staticBodiesPendingAdd;
    std::unique_ptr<ComponentGroupOnAdd<DynamicBodyComponent>> dynamicBodiesPendingAdd;
    std::unique_ptr<ComponentGroupOnAdd<CollisionShapeComponent>> shapesPendingAdd;
    std::unique_ptr<ComponentGroupOnAdd<BoxCharacterControllerComponent>> boxCCTsPendingAdd;
    std::unique_ptr<ComponentGroupOnAdd<CapsuleCharacterControllerComponent>> capsuleCCTsPendingAdd;

    // Set of entities whose render components are ready to be used for creating physx shape
    UnorderedSet<Entity*> readyRenderedEntities;

    // Contains all the entities which participate in physics simulation
    UnorderedSet<Entity*> physicsEntities;
};
} // namespace DAVA
