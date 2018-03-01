#pragma once

#include "Components/AI/BotTaskComponent.h"
#include "Components/AI/InvaderBehaviorComponent.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/ComponentGroup.h>
#include <Scene3D/Scene.h>
#include <Utils/Random.h>

using namespace DAVA;

class InvaderBehaviorSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(InvaderBehaviorSystem, SceneSystem);

    explicit InvaderBehaviorSystem(Scene* scene);

    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;

private:
    void ProcessPendingAgents();

    bool UpdateScenario(InvaderBehaviorComponent* agent, float32 timeElapsed);

    void ClearCurrentTask(InvaderBehaviorComponent* agent);
    void SetCurrentTask(InvaderBehaviorComponent* agent, BotTaskComponent* task);

    void InitPosition(InvaderBehaviorComponent* agent);
    BotTaskComponent* CreateInitialTask(InvaderBehaviorComponent *agent);
    BotTaskComponent* CreateNextTask(InvaderBehaviorComponent *agent);

    Vector<InvaderBehaviorComponent*> agents;
    Vector<InvaderBehaviorComponent*> pendingAgents;
};
