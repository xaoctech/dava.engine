#pragma once

#include "Scene3D/Systems/BaseSimulationSystem.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class Scene;
class Entity;
class Component;
class NetworkTimeSingleComponent;
}

class HealthComponent;
class DamageComponent;

class DamageSystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(DamageSystem, DAVA::SceneSystem);

    DamageSystem(DAVA::Scene* scene);

    void PrepareForRemove() override{};
    void ProcessFixed(DAVA::float32 timeElapsed) override;

private:
    void ApplyDamage(HealthComponent* health, DamageComponent* damage);
    DAVA::NetworkTimeSingleComponent* timeComp;
};
