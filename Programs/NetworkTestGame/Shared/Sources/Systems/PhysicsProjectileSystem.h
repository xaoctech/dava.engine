#pragma once

#include <Base/BaseTypes.h>
#include "Entity/SceneSystem.h"
#include "Scene3D/Systems/BaseSimulationSystem.h"

namespace DAVA
{
class Entity;
}

class PhysicsProjectileComponent;

class PhysicsProjectileSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(PhysicsProjectileSystem, DAVA::BaseSimulationSystem);

    PhysicsProjectileSystem(DAVA::Scene* scene);

    void Simulate(DAVA::Entity* entity) override;
    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    void NextState(DAVA::Entity* entity, PhysicsProjectileComponent* projectileComponent);

private:
    DAVA::UnorderedMap<DAVA::Entity*, DAVA::float32> grenadeDetonationTimers;
};
