#pragma once

#include "Components/SingleComponents/BattleOptionsSingleComponent.h"

#include <Base/UnordererSet.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>

namespace DAVA
{
class Scene;
}

class EffectQueueSingleComponent;

// System that responsible for invoking attack actions (i.e. raycasting, spawning bullets etc.)
class ShooterPlayerAttackSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterPlayerAttackSystem, DAVA::BaseSimulationSystem);

    ShooterPlayerAttackSystem(DAVA::Scene* scene);
    void ProcessFixed(DAVA::float32 dt) override;
    void PrepareForRemove() override;

    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration);
    void ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration);

private:
    void SpawnBullet(DAVA::Entity* player, DAVA::uint32 clientFrameId) const;
    void RaycastAttack(DAVA::Entity* player, DAVA::uint32 clientFrameId) const;
    void RocketAttack(DAVA::Entity* player, DAVA::uint32 clientFrameId, bool multirocket) const;

private:
    BattleOptionsSingleComponent* optionsComp = nullptr;
    DAVA::EntityGroup* entityGroup = nullptr;

    EffectQueueSingleComponent* effectQueue = nullptr;
};
