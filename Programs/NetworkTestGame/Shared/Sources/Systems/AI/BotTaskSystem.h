#pragma once

#include "Components/AI/AttackTaskComponent.h"
#include "Components/AI/MoveToPointTaskComponent.h"
#include "Components/AI/WaitTaskComponent.h"
#include "Components/AI/CompositeTaskComponent.h"
#include "Components/AI/RandomMovementTaskComponent.h"

#include <Base/BaseTypes.h>
#include <Base/Vector.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>
#include <Components/AI/ShooterBehaviorComponent.h>

using namespace DAVA;

class BotTaskSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(BotTaskSystem, SceneSystem);

    BotTaskSystem(Scene* scene);

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

    bool GetShotParams(float& outYawCorrection, float& outDist, float32 timeElapsed, AttackTaskComponent* task);

    void ProcessTask(float32 timeElapsed, AttackTaskComponent* task);
    void ProcessTask(float32 timeElapsed, MoveToPointTaskComponent* task);
    void ProcessTask(float32 timeElapsed, WaitTaskComponent* task);
    void ProcessTask(float32 timeElapsed, CompositeTaskComponent* task);
    void ProcessTask(float32 timeElapsed, RandomMovementTaskComponent* task);

    TaskStorage<
    AttackTaskComponent,
    MoveToPointTaskComponent,
    WaitTaskComponent,
    CompositeTaskComponent,
    RandomMovementTaskComponent> taskStorage;

    Vector<FastName> currentDigitalActions;
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
