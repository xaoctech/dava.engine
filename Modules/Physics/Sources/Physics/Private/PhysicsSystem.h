#pragma once

#include <Entity/SceneSystem.h>

namespace physx
{
class PxScene;
}

namespace DAVA
{
class Scene;
class PhysicsSystem : public SceneSystem
{
public:
    PhysicsSystem(Scene* scene);
    ~PhysicsSystem() override;

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
};
} // namespace DAVA