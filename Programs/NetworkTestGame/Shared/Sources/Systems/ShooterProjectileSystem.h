#pragma once

#include <Base/UnordererSet.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

namespace DAVA
{
class Scene;
}

class ShooterProjectileComponent;

class ShooterProjectileSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterProjectileSystem, DAVA::BaseSimulationSystem);

    ShooterProjectileSystem(DAVA::Scene* scene);
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void ProcessFixed(DAVA::float32 dt) override;
    void Simulate(DAVA::Entity* entity) override;
    void PrepareForRemove() override;

private:
    DAVA::UnorderedSet<ShooterProjectileComponent*> projectileComponents;
};