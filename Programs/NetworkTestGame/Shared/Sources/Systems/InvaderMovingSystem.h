#pragma once

#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "Components/SingleComponents/StatsLoggingSingleComponent.h"

#include <Base/BaseTypes.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>

namespace DAVA
{
class Scene;
class Entity;
}

namespace InvaderMovingSystemDetail
{
static const DAVA::FastName UP("UP");
static const DAVA::FastName DOWN("DOWN");
static const DAVA::FastName LEFT("LEFT");
static const DAVA::FastName RIGHT("RIGHT");
static const DAVA::FastName ACCELERATE("ACCELERATE");
static const DAVA::FastName TELEPORT("TELEPORT");

DAVA::Vector2 GetNormalizedTeleportPosition(const DAVA::Vector2& pos);
}

class InvaderMovingSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(InvaderMovingSystem, DAVA::BaseSimulationSystem);

    explicit InvaderMovingSystem(DAVA::Scene* scene);

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions,
                             DAVA::uint32 clientFrameId, DAVA::float32 duration);
    void ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions,
                            DAVA::uint32 clientFrameId, DAVA::float32 duration);

private:
    DAVA::EntityGroup* entityGroup = nullptr;
};
