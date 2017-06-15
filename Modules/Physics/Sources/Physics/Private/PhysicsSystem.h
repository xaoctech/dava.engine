#pragma once

#include <Entity/SceneSystem.h>

namespace physx
{
class PxScene;
}

namespace DAVA
{
class Scene;
class PhysicsComponent;

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

private:
    bool FetchResults(bool block);

private:
    void* simulationBlock = nullptr;
    uint32 simulationBlockSize = 0;

    bool isSimulationEnabled = true;
    bool isSimulationRunning = false;
    physx::PxScene* physicsScene = nullptr;

    Vector<PhysicsComponent*> components;
    Vector<PhysicsComponent*> pendingAddComponents;
};
} // namespace DAVA