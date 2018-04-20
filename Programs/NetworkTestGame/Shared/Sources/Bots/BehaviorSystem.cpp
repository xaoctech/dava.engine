#include "BehaviorSystem.h"

#include "Systems/InvaderMovingSystem.h"

#include <Logger/Logger.h>
#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkClientConnectionSingleComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedObject.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(BehaviorSystem)
{
    ReflectionRegistrator<BehaviorSystem>::Begin()
    .End();
}

BehaviorSystem::BehaviorSystem(Scene* scene, const ComponentMask& requiredComponents,
                               ComponentGroup<BehaviorComponent>* agents_)
    : SceneSystem(scene, ComponentMask())
    , agents(agents_)
    , pendingAgents(scene->AquireComponentGroupOnAdd(agents_, this))
{
}

void BehaviorSystem::ProcessFixed(float32 timeElapsed)
{
    const NetworkClientConnectionSingleComponent* networkConnectionComponent = GetScene()->GetSingleComponentForRead<NetworkClientConnectionSingleComponent>(this);
    if (!networkConnectionComponent->IsConnected())
    {
        return;
    }

    const NetworkTimeSingleComponent* networkTimeComponent = GetScene()->GetSingleComponentForRead<NetworkTimeSingleComponent>(this);
    if (!networkTimeComponent->IsInitialized())
    {
        return;
    }

    // do some preliminary checks to be sure we may start
    if (!CheckStartConditions(timeElapsed))
    {
        return;
    }

    // init pending agents
    for (BehaviorComponent* pendingAgent : pendingAgents->components)
    {
        pendingAgent->isActor = IsClientOwner(pendingAgent->GetEntity());
        ProcessPending(pendingAgent);
    }
    pendingAgents->components.clear();

    // process agents
    for (BehaviorComponent* agent : agents->components)
    {
        if (!agent->isActor)
        {
            continue;
        }

        // switch scenarios
        bool scenarioChanged = UpdateScenario(agent, timeElapsed);

        // manage tasks
        BotTaskComponent* currentTask = agent->currentTask;
        if (!currentTask || scenarioChanged)
        {
            InitPosition(agent);
            SetCurrentTask(agent, CreateInitialTask(agent));
        }
        else if (currentTask->GetStatus() != BotTaskStatus::IN_PROGRESS)
        {
            SetCurrentTask(agent, CreateNextTask(agent));
        }

        agent->lifetime += timeElapsed;
    }
}

bool BehaviorSystem::CheckStartConditions(float32 timeElapsed)
{
    // trivial implementation, override it in specific behavior system if needed
    return true;
}

bool BehaviorSystem::UpdateScenario(BehaviorComponent* actor, float32 timeElapsed)
{
    // trivial implementation, override it in specific behavior system if needed
    return false;
}

void BehaviorSystem::ClearCurrentTask(BehaviorComponent* agent)
{
    if (agent->currentTask)
    {
        agent->currentTask->TraverseTaskTree([](BotTaskComponent* task) { task->GetEntity()->RemoveComponent(task); });
        agent->currentTask = nullptr;
        Logger::Debug("[BehaviorSystem] Task cleared for player %d",
                      agent->GetEntity()->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID());
    }
}

void BehaviorSystem::SetCurrentTask(BehaviorComponent* agent, BotTaskComponent* task)
{
    ClearCurrentTask(agent);
    if (task)
    {
        Entity* entity = agent->GetEntity();
        task->TraverseTaskTree([&entity](BotTaskComponent* task) { entity->AddComponent(task); });
        agent->currentTask = task;
        Logger::Debug("[BehaviorSystem] Task %s given to player %d",
                      ReflectedObject(task).GetReflectedType()->GetPermanentName().c_str(),
                      agent->GetEntity()->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID());
    }
}
