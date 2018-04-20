#pragma once

#include "BattleRoyaleBehaviorComponent.h"
#include "BehaviorSystem.h"
#include "BotTaskComponent.h"
#include "ShooterConstants.h"
#include "Components/ShooterRoleComponent.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/ComponentGroup.h>
#include <Scene3D/Scene.h>
#include <Utils/Random.h>

namespace DAVA
{
class ShooterBattleRoyaleBehaviorSystem final : public BehaviorSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterBattleRoyaleBehaviorSystem, BehaviorSystem);

    explicit ShooterBattleRoyaleBehaviorSystem(Scene* scene);

private:
    bool CheckStartConditions(float32 timeElapsed) override;
    void ProcessPending(BehaviorComponent* agent) override;
    void InitPosition(BehaviorComponent* agent) override;
    BotTaskComponent* CreateInitialTask(BehaviorComponent* agent) override;
    BotTaskComponent* CreateNextTask(BehaviorComponent* agent) override;

    BotTaskComponent* CreateNextAttackTask(BattleRoyaleBehaviorComponent* actor);
    BotTaskComponent* CreateNextMoveTask(BattleRoyaleBehaviorComponent* actor);

    float32 waitTime = 0.f;

    ComponentGroup<ShooterRoleComponent>* roles = nullptr;
};
}
