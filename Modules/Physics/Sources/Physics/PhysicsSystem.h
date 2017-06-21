#pragma once

#include <Entity/SceneSystem.h>

namespace physx
{
class PxScene;
class PxRigidActor;
class PxShape;
}

namespace DAVA
{
class Scene;
class Physics;
class PhysicsComponent;
class CollisionShapeComponent;

class PhysicsSystem : public SceneSystem
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

    void SetDrawDebugInfo(bool drawDebugInfo);
    bool IsDrawDebugInfo() const;

private:
    bool FetchResults(bool block);

    void DrawDebugInfo();
    void DrawDebugObject();

    void InitNewObjects();
    void AttachShape(Entity* entity, physx::PxRigidActor* actor);
    physx::PxShape* CreateShape(CollisionShapeComponent* component, Physics* physics);

    void SyncTransformToPhysx();

private:
    friend class PhysicsSystemPrivate; // for tests only

    void* simulationBlock = nullptr;
    uint32 simulationBlockSize = 0;

    bool isSimulationEnabled = true;
    bool isSimulationRunning = false;
    physx::PxScene* physicsScene = nullptr;

    Vector<PhysicsComponent*> physicsComponents;
    Vector<PhysicsComponent*> pendingAddPhysicsComponents;

    Vector<CollisionShapeComponent*> collisionComponents;
    Vector<CollisionShapeComponent*> pendingAddCollisionComponents;

    bool drawDebugInfo = false;
};
} // namespace DAVA