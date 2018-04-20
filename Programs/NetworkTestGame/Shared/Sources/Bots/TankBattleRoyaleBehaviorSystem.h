#pragma once

#include "BattleRoyaleBehaviorComponent.h"
#include "BehaviorSystem.h"
#include "BotTaskComponent.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Utils/Random.h>

namespace DAVA
{
class BattleRoyaleBehaviorComponent;

class TankBattleRoyaleBehaviorSystem final : public BehaviorSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(TankBattleRoyaleBehaviorSystem, BehaviorSystem);

    explicit TankBattleRoyaleBehaviorSystem(Scene* scene);

private:
    void ProcessPending(BehaviorComponent* agent) override;
    void InitPosition(BehaviorComponent* agent) override;
    BotTaskComponent* CreateInitialTask(BehaviorComponent* agent) override;
    BotTaskComponent* CreateNextTask(BehaviorComponent* agent) override;

    BotTaskComponent* CreateNextMoveTask(BattleRoyaleBehaviorComponent* agent);
    BotTaskComponent* CreateNextAttackTask(BattleRoyaleBehaviorComponent* agent);
};
}
