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

namespace InvaderAttackSystemDetail
{
static const DAVA::FastName FIRST_SHOOT("FIRST_SHOOT");
}

class InvaderAttackSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(InvaderAttackSystem, DAVA::BaseSimulationSystem);

    explicit InvaderAttackSystem(DAVA::Scene* scene);

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions,
                             DAVA::uint32 clientFrameId, DAVA::float32 duration);
    void ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions,
                            DAVA::uint32 clientFrameId, DAVA::float32 duration);

private:
    DAVA::Entity* Shoot(const DAVA::Vector3& shootStart, const DAVA::Vector3& shootDirection) const;
    DAVA::Entity* ShootInPast(const DAVA::Entity* shooter, const DAVA::Vector3& shootStart, const DAVA::Vector3& shootDirection,
                              DAVA::uint32 clientFrameId) const;
    void LogStrike(const DAVA::Entity* shooter, const DAVA::Entity* victim, DAVA::uint32 clientFrameID) const;

private:
    DAVA::Entity* ownerEntity = nullptr;

    BattleOptionsSingleComponent* optionsComp = nullptr;
    StatsLoggingSingleComponent* statsComp = nullptr;

    DAVA::EntityGroup* entityGroup = nullptr;
};
