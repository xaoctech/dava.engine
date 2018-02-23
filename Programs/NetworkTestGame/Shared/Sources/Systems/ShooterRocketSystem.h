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
class ShooterRocketComponent;

class ShooterRocketSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterRocketSystem, DAVA::BaseSimulationSystem);

    ShooterRocketSystem(DAVA::Scene* scene);
    ~ShooterRocketSystem();

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void Simulate(DAVA::Entity* entity) override;
    void PrepareForRemove() override{};

protected:
    const bool HAS_MISPREDICTION = false;
    const bool SELF_DAMAGE = false;

    DAVA::Entity* GetRocketModel();
    bool IsSimulated(DAVA::Entity* rocket);
    void FillRocket(DAVA::Entity* rocket);
    const DAVA::Entity* GetTarget(DAVA::Entity* rocket, DAVA::Entity* shooter);
    DAVA::Entity* SpawnSubRocket(DAVA::Entity* shooter, const DAVA::Entity* target, const DAVA::FrameActionID& shootActionId);
    void Colorize(DAVA::Entity* rocket);
    void Colorize(DAVA::Entity* model, const DAVA::Color& color);

    DAVA::UnorderedSet<DAVA::Entity*> pendingEntities;
    DAVA::UnorderedSet<DAVA::Entity*> destroyedEntities;
    DAVA::Entity* rocketModel = nullptr;

    DAVA::NetworkEntitiesSingleComponent* entitiesComp = nullptr;
};
