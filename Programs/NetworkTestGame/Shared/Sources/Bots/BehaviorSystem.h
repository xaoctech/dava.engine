#pragma once

#include "BotTaskComponent.h"
#include "BehaviorComponent.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/ComponentGroup.h>
#include <Scene3D/Scene.h>
#include <Utils/Random.h>

namespace DAVA
{
class BehaviorSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(BehaviorSystem, SceneSystem);

    BehaviorSystem(Scene* scene, const ComponentMask& requiredComponents, ComponentGroup<BehaviorComponent>* agents);
    virtual ~BehaviorSystem() = default;

    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    void ClearCurrentTask(BehaviorComponent* agent);
    void SetCurrentTask(BehaviorComponent* agent, BotTaskComponent* task);

    virtual bool CheckStartConditions(float32 timeElapsed);
    virtual void ProcessPending(BehaviorComponent* agent) = 0;
    virtual bool UpdateScenario(BehaviorComponent* agent, float32 timeElapsed);
    virtual void InitPosition(BehaviorComponent* agent) = 0;
    virtual BotTaskComponent* CreateInitialTask(BehaviorComponent* agent) = 0;
    virtual BotTaskComponent* CreateNextTask(BehaviorComponent* agent) = 0;

protected:
    ComponentGroup<BehaviorComponent>* agents = nullptr;
    ComponentGroupOnAdd<BehaviorComponent>* pendingAgents = nullptr;
};
}
