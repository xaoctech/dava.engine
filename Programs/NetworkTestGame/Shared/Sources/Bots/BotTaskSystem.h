#pragma once

#include "BattleRoyaleBehaviorComponent.h"
#include "InvaderBehaviorComponent.h"
#include "CommonTaskComponents.h"
#include "InvaderTaskComponents.h"
#include "ShooterTaskComponents.h"
#include "TankTaskComponents.h"
#include "Components/ShooterAimComponent.h"
#include "Scene3D/Components/TransformComponent.h"

#include <Base/BaseTypes.h>
#include <Base/Vector.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
class BotTaskSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(BotTaskSystem, SceneSystem);

    explicit BotTaskSystem(Scene* scene);

    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;

private:
    template <class... Ts>
    class TaskStorage
    {
    public:
        TaskStorage(BotTaskSystem* p)
            : processor(p)
        {
        }
        void DispatchTasks(float32 timeElapsed)
        {
        }
        void Add(Component* task)
        {
        }
        void Remove(Component* task)
        {
        }

    protected:
        BotTaskSystem* processor;
    };

    template <class T, class... Ts>
    class TaskStorage<T, Ts...> : public TaskStorage<Ts...>
    {
    public:
        TaskStorage(BotTaskSystem* p)
            : TaskStorage<Ts...>(p)
        {
        }
        void DispatchTasks(float32 timeElapsed);
        void Add(Component* task);
        void Remove(Component* task);

    private:
        Vector<T*> tasks;
    };

    bool GetShotParams(float& outYawCorrection, float& outDist, float32 timeElapsed, TankAttackTaskComponent* task);
    void RotateTowardsTarget(const ShooterAimComponent& aimComponent, const Vector3& targetPosition);
    BotTaskStatus UpdateAttackTaskStatus(Entity* target);

    void ProcessTask(float32 timeElapsed, WaitTaskComponent* task);
    void ProcessTask(float32 timeElapsed, CompositeTaskComponent* task);

    void ProcessTask(float32 timeElapsed, TankAttackTaskComponent* task);
    void ProcessTask(float32 timeElapsed, TankMoveToPointTaskComponent* task);
    void ProcessTask(float32 timeElapsed, TankRandomMovementTaskComponent* task);

    void ProcessTask(float32 timeElapsed, InvaderSlideToBorderTaskComponent* task);
    void ProcessTask(float32 timeElapsed, InvaderWagToBorderTaskComponent* task);
    void ProcessTask(float32 timeElapsed, InvaderDodgeCenterTaskComponent* task);
    void ProcessTask(float32 timeElapsed, InvaderShootIfSeeingTargetTaskComponent* task);

    void ProcessTask(float32 timeElapsed, ShooterAttackStandingStillTaskComponent* task);
    void ProcessTask(float32 timeElapsed, ShooterAttackPursuingTargetTaskComponent* task);
    void ProcessTask(float32 timeElapsed, ShooterAttackCirclingAroundTaskComponent* task);
    void ProcessTask(float32 timeElapsed, ShooterAttackWaggingTaskComponent* task);
    void ProcessTask(float32 timeElapsed, ShooterMoveToPointShortestTaskComponent* task);
    void ProcessTask(float32 timeElapsed, ShooterMoveToPointWindingTaskComponent* task);
    void ProcessTask(float32 timeElapsed, ShooterHangAroundTaskComponent* task);
    void ProcessTask(float32 timeElapsed, ShooterDriveTaskComponent* task);

    TaskStorage<
    WaitTaskComponent,
    CompositeTaskComponent,
    TankAttackTaskComponent,
    TankMoveToPointTaskComponent,
    TankRandomMovementTaskComponent,
    InvaderSlideToBorderTaskComponent,
    InvaderWagToBorderTaskComponent,
    InvaderDodgeCenterTaskComponent,
    InvaderShootIfSeeingTargetTaskComponent,
    ShooterAttackStandingStillTaskComponent,
    ShooterAttackPursuingTargetTaskComponent,
    ShooterAttackCirclingAroundTaskComponent,
    ShooterAttackWaggingTaskComponent,
    ShooterMoveToPointShortestTaskComponent,
    ShooterMoveToPointWindingTaskComponent,
    ShooterHangAroundTaskComponent,
    ShooterDriveTaskComponent> taskStorage;

    Vector<FastName> currentDigitalActions;
    Vector<std::pair<FastName, Vector2>> currentAnalogActions;
};

template <class T, class... Ts>
inline void BotTaskSystem::TaskStorage<T, Ts...>::DispatchTasks(float32 timeElapsed)
{
    for (T* task : tasks)
    {
        TaskStorage<>::processor->ProcessTask(timeElapsed, task);
    }
    TaskStorage<Ts...>::DispatchTasks(timeElapsed);
}

template <class T, class... Ts>
inline void BotTaskSystem::TaskStorage<T, Ts...>::Add(Component* task)
{
    T* concreteTask = dynamic_cast<T*>(task);
    if (concreteTask)
    {
        concreteTask->status = BotTaskStatus::IN_PROGRESS;
        tasks.push_back(concreteTask);
    }
    else
    {
        TaskStorage<Ts...>::Add(task);
    }
}

template <class T, class... Ts>
inline void BotTaskSystem::TaskStorage<T, Ts...>::Remove(Component* task)
{
    T* concreteTask = dynamic_cast<T*>(task);
    if (concreteTask)
    {
        auto found = std::find(tasks.begin(), tasks.end(), concreteTask);
        if (found != tasks.end())
        {
            tasks.erase(found);
        }
    }
    else
    {
        TaskStorage<Ts...>::Remove(task);
    }
}
}