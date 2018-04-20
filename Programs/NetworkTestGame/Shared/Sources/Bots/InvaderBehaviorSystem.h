#pragma once

#include "BotTaskComponent.h"
#include "BehaviorSystem.h"
#include "InvaderBehaviorComponent.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/ComponentGroup.h>
#include <Scene3D/Scene.h>
#include <Utils/Random.h>

namespace DAVA
{
class InvaderBehaviorSystem final : public BehaviorSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(InvaderBehaviorSystem, BehaviorSystem);

    explicit InvaderBehaviorSystem(Scene* scene);

private:
    bool CheckStartConditions(float32 timeElapsed) override;
    void ProcessPending(BehaviorComponent* agent) override;
    bool UpdateScenario(BehaviorComponent* agent, float32 timeElapsed) override;
    void InitPosition(BehaviorComponent* agent) override;
    BotTaskComponent* CreateInitialTask(BehaviorComponent* agent) override;
    BotTaskComponent* CreateNextTask(BehaviorComponent* agent) override;

    bool allRolesPresent = false;
    uint8 nextRole = InvaderBehaviorComponent::Role::OBSERVER;
};
}
