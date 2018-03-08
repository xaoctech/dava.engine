#pragma once

#include <NetworkCore/NetworkTypes.h>

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

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

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void SimulateRocket(DAVA::Entity* entity);

protected:
    DAVA::Entity* GetRocketModel() const;

    mutable DAVA::Entity* rocketModel = nullptr;

    const bool HAS_MISPREDICTION = false;
    const bool SELF_DAMAGE = false;

    void FillRocket(DAVA::Entity* rocket);
    const DAVA::Entity* GetTarget(DAVA::Entity* rocket, DAVA::Entity* shooter);
    DAVA::Entity* SpawnSubRocket(DAVA::Entity* shooter, const DAVA::Entity* target, DAVA::NetworkID rocketId);
    void Colorize(DAVA::Entity* rocket);

    DAVA::EntityGroup* entityGroup = nullptr;
    DAVA::EntityGroupOnAdd* pendingEntities = nullptr;

    DAVA::UnorderedSet<DAVA::Entity*> destroyedEntities;

    DAVA::NetworkEntitiesSingleComponent* entitiesComp = nullptr;
};
