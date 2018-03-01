#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Math/Color.h"
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

    void ProcessFixed(DAVA::float32 timeElapsed) override;

    void ReSimulationStart() override;
    void ReSimulationEnd() override;

    void SimulateRocket(DAVA::Entity* entity);
    void PrepareForRemove() override{};

protected:
    const bool HAS_MISPREDICTION = false;
    const bool SELF_DAMAGE = false;

    DAVA::Entity* GetRocketModel();
    void FillRocket(DAVA::Entity* rocket);
    const DAVA::Entity* GetTarget(DAVA::Entity* rocket, DAVA::Entity* shooter);
    DAVA::Entity* SpawnSubRocket(DAVA::Entity* shooter, const DAVA::Entity* target, const DAVA::FrameActionID& shootActionId);
    void Colorize(DAVA::Entity* rocket);
    void Colorize(DAVA::Entity* model, const DAVA::Color& color);

    DAVA::EntityGroup* entityGroup = nullptr;
    DAVA::EntityGroupOnAdd pendingEntities;
    struct
    {
        DAVA::Vector<DAVA::Entity*> entities;
    } pendingBackUp;
    DAVA::UnorderedSet<DAVA::Entity*> destroyedEntities;
    DAVA::Entity* rocketModel = nullptr;

    DAVA::NetworkEntitiesSingleComponent* entitiesComp = nullptr;
};
