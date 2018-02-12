#pragma once

#include "Components/AI/ShooterBehaviorComponent.h"
#include "Components/AI/BotTaskComponent.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>
#include <Utils/Random.h>

using namespace DAVA;

class ShooterBehaviorSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterBehaviorSystem, SceneSystem);

    ShooterBehaviorSystem(Scene* scene);

    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

private:
    void ProcessPendingAgents();
    void ClearCurrentTask(ShooterBehaviorComponent* agent);
    void SetCurrentTask(ShooterBehaviorComponent* agent, BotTaskComponent* task);

    BotTaskComponent* CreateInititalTask(ShooterBehaviorComponent* agent);
    BotTaskComponent* CreateNextTask(ShooterBehaviorComponent* agent);
    BotTaskComponent* CreateNextMoveTask(ShooterBehaviorComponent* agent);
    BotTaskComponent* CreateNextAttackTask(ShooterBehaviorComponent* agent);

    Random localRandom;

    Vector<ShooterBehaviorComponent*> agents;
    Vector<ShooterBehaviorComponent*> pendingAgents;
    DAVA::ComponentGroup<ShooterBehaviorComponent>* otherAgents = nullptr;
};
