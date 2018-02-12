#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/Systems/BaseSimulationSystem.h"

namespace DAVA
{
class Entity;
class FrameActionID;
class NetworkEntitiesSingleComponent;
}
class ExplosiveRocketComponent;

class ExplosiveRocketSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ExplosiveRocketSystem, DAVA::BaseSimulationSystem);

    ExplosiveRocketSystem(DAVA::Scene* scene);
    ~ExplosiveRocketSystem();

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void Simulate(DAVA::Entity* entity) override;

protected:
    const bool HAS_MISPREDICTION = false;
    const bool SELF_DAMAGE = false;

    DAVA::Entity* GetRocketModel() const;
    bool IsSimulated(DAVA::Entity* rocket);
    void FillRocket(DAVA::Entity* rocket);
    const DAVA::Entity* GetTarget(DAVA::Entity* rocket, DAVA::Entity* shooter);
    DAVA::Entity* SpawnSubRocket(DAVA::Entity* shooter, const DAVA::Entity* target, const DAVA::FrameActionID& shootActionId);
    void Colorize(DAVA::Entity* rocket);

    DAVA::UnorderedSet<DAVA::Entity*> pendingEntities;
    DAVA::UnorderedSet<DAVA::Entity*> destroyedEntities;
    mutable DAVA::Entity* rocketModel = nullptr;

    DAVA::NetworkEntitiesSingleComponent* entitiesComp = nullptr;
};
