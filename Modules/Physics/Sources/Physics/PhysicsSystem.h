#pragma once

#include <Entity/SceneSystem.h>
#include <Math/Vector.h>
#include <Base/BaseTypes.h>

#include <physx/PxQueryReport.h>

namespace physx
{
class PxScene;
class PxRigidActor;
class PxShape;
class PxControllerManager;
}

namespace DAVA
{
class Vector3;
class Scene;
class PhysicsModule;
class PhysicsComponent;
class CollisionShapeComponent;
class PhysicsGeometryCache;
class CharacterControllerComponent;

class PhysicsSystem final : public SceneSystem
{
public:
    PhysicsSystem(Scene* scene);
    ~PhysicsSystem() override;

    void RegisterEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;

    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;

    void Process(float32 timeElapsed) override;

    void SetSimulationEnabled(bool isEnabled);
    bool IsSimulationEnabled() const;

    void SetDebugDrawEnabled(bool drawDebugInfo);
    bool IsDebugDrawEnabled() const;

    void ScheduleUpdate(PhysicsComponent* component);
    void ScheduleUpdate(CollisionShapeComponent* component);
    void ScheduleUpdate(CharacterControllerComponent* component);

    bool Raycast(const Vector3& origin, const Vector3& direction, float32 distance, physx::PxRaycastCallback& callback);

private:
    bool FetchResults(bool waitForFetchFinish);

    void DrawDebugInfo();

    void InitNewObjects();
    void AttachShape(Entity* entity, PhysicsComponent* bodyComponent, const Vector3& scale);
    void AttachShape(PhysicsComponent* bodyComponent, CollisionShapeComponent* shapeComponent, const Vector3& scale);

    void ReleaseShape(CollisionShapeComponent* component);
    physx::PxShape* CreateShape(CollisionShapeComponent* component, PhysicsModule* physics);

    void SyncTransformToPhysx();
    void SyncEntityTransformToPhysx(Entity* entity);
    void UpdateComponents();

    void MoveCharacterControllers(float32 timeElapsed);

private:
    friend class PhysicsSystemPrivate; // for tests only

    void* simulationBlock = nullptr;
    uint32 simulationBlockSize = 0;

    bool isSimulationEnabled = true;
    bool isSimulationRunning = false;
    physx::PxScene* physicsScene = nullptr;
    physx::PxControllerManager* controllerManager = nullptr;
    PhysicsGeometryCache* geometryCache = nullptr;

    Vector<PhysicsComponent*> physicsComponents;
    Vector<PhysicsComponent*> pendingAddPhysicsComponents;

    Vector<CollisionShapeComponent*> collisionComponents;
    Vector<CollisionShapeComponent*> pendingAddCollisionComponents;

    Vector<CharacterControllerComponent*> characterControllerComponents;
    Vector<CharacterControllerComponent*> pendingAddCharacterControllerComponents;

    UnorderedMap<Entity*, Vector<CollisionShapeComponent*>> waitRenderInfoComponents;

    Set<PhysicsComponent*> physicsComponensUpdatePending;
    Set<CollisionShapeComponent*> collisionComponentsUpdatePending;
    Set<CharacterControllerComponent*> characterControllerComponentsUpdatePending;

    bool drawDebugInfo = false;
};
} // namespace DAVA