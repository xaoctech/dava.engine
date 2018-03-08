#include "InvaderBehaviorSystem.h"

#include "Components/HealthComponent.h"
#include "Components/AI/CompositeTaskComponent.h"
#include "Components/AI/DodgeCenterTaskComponent.h"
#include "Components/AI/ShootIfSeeingTargetTaskComponent.h"
#include "Components/AI/SlideToBorderTaskComponent.h"
#include "Components/AI/WagToBorderTaskComponent.h"
#include "Components/AI/WaitTaskComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "Scene3D/Components/SingleComponents/ActionsSingleComponent.h"
#include "Systems/InvaderMovingSystem.h"

#include <Logger/Logger.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedObject.h>

DAVA_VIRTUAL_REFLECTION_IMPL(InvaderBehaviorSystem)
{
    ReflectionRegistrator<InvaderBehaviorSystem>::Begin()[M::Tags("bot", "invaderbot")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &InvaderBehaviorSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 15.1f)]
    .End();
}

namespace InvaderBehaviorSystemDetails
{
static const float32 DELAY_DURATION = 0.5f;
static const float32 SCENARIO_DURATION = 2.5f;
}

InvaderBehaviorSystem::InvaderBehaviorSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<InvaderBehaviorComponent>() | ComponentUtils::MakeMask<NetworkReplicationComponent>())
{
}

void InvaderBehaviorSystem::ProcessFixed(float32 timeElapsed)
{
    ProcessPendingAgents();

    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingleComponent<NetworkTimeSingleComponent>();
    if (!netTimeComp->IsInitialized())
    {
        return;
    }

    if (agents.empty())
    {
        return;
    }

    for (InvaderBehaviorComponent* agent : agents)
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
    }
}

void InvaderBehaviorSystem::RegisterComponent(Entity* entity, Component* component)
{
    InvaderBehaviorComponent* agent = dynamic_cast<InvaderBehaviorComponent*>(component);
    if (agent)
    {
        pendingAgents.push_back(agent);
        Logger::Debug("[InvaderBehavior] New invader registered, total pending %d", pendingAgents.size());
    }
}

void InvaderBehaviorSystem::ProcessPendingAgents()
{
    using namespace InvaderBehaviorSystemDetails;

    // wait till all roles are present
    if (pendingAgents.size() < InvaderBehaviorComponent::ROLES_COUNT)
    {
        return;
    }

    // give roles in same order on all clients
    std::sort(pendingAgents.begin(), pendingAgents.end(),
              [](const InvaderBehaviorComponent* lhs, const InvaderBehaviorComponent* rhs)
              {
                  NetworkReplicationComponent* lhsReplComp = lhs->GetEntity()->GetComponent<NetworkReplicationComponent>();
                  NetworkReplicationComponent* rhsReplComp = rhs->GetEntity()->GetComponent<NetworkReplicationComponent>();
                  return lhsReplComp->GetNetworkPlayerID() < rhsReplComp->GetNetworkPlayerID();
              });

    uint8 lastGivenRole = InvaderBehaviorComponent::Role::OBSERVER;
    for (InvaderBehaviorComponent* agent : pendingAgents)
    {
        agent->role = static_cast<InvaderBehaviorComponent::Role>(lastGivenRole++ % InvaderBehaviorComponent::ROLES_COUNT);
        agent->timeBeforeScenarioSwitch = SCENARIO_DURATION;
        agents.push_back(agent);

        Logger::Debug("[InvaderBehavior] Setting role %s for invader id %d",
                      InvaderBehaviorComponent::roleNames[agent->role].c_str(),
                      agent->GetEntity()->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID());
    }

    pendingAgents.clear();
}

void InvaderBehaviorSystem::UnregisterComponent(Entity* entity, Component* component)
{
    InvaderBehaviorComponent* agent = dynamic_cast<InvaderBehaviorComponent*>(component);
    if (!agent)
    {
        return;
    }

    auto found = std::find(agents.begin(), agents.end(), agent);
    if (found != agents.end())
    {
        ClearCurrentTask(agent);
        agents.erase(found);
    }

    auto foundPending = std::find(pendingAgents.begin(), pendingAgents.end(), agent);
    if (foundPending != pendingAgents.end())
    {
        pendingAgents.erase(found);
    }
}

bool InvaderBehaviorSystem::UpdateScenario(InvaderBehaviorComponent* actor, float32 timeElapsed)
{
    using namespace InvaderBehaviorSystemDetails;

    actor->timeBeforeScenarioSwitch -= timeElapsed;
    if (actor->timeBeforeScenarioSwitch <= 0.f)
    {
        actor->syncCounter++;

        actor->currentScenario++;
        actor->currentScenario %= InvaderBehaviorComponent::SCENARIOS_COUNT;
        actor->timeBeforeScenarioSwitch = SCENARIO_DURATION;
        Logger::Debug("[InvaderBehavior] Switching scenario to %s",
                      InvaderBehaviorComponent::scenarioNames[actor->currentScenario].c_str());
        return true;
    }

    return false;
}

void InvaderBehaviorSystem::InitPosition(InvaderBehaviorComponent* actor)
{
    float32 offsetX = 0.f;
    float32 offsetY = 0.f;
    switch (actor->role)
    {
    case InvaderBehaviorComponent::Role::SHOOTER:
    {
        offsetX = (actor->currentScenario == InvaderBehaviorComponent::Scenario::SLIDING_SHOOTER ||
                   actor->currentScenario == InvaderBehaviorComponent::Scenario::SLIDING_BOTH ||
                   actor->currentScenario == InvaderBehaviorComponent::Scenario::WAGGING_SHOOTER ||
                   actor->currentScenario == InvaderBehaviorComponent::Scenario::WAGGING_BOTH) ?
        -30.f :
        0.f;
        offsetY = -30.f;
        break;
    }
    case InvaderBehaviorComponent::Role::TARGET:
    {
        offsetX = (actor->currentScenario == InvaderBehaviorComponent::Scenario::SLIDING_TARGET ||
                   actor->currentScenario == InvaderBehaviorComponent::Scenario::WAGGING_TARGET ||
                   actor->currentScenario == InvaderBehaviorComponent::Scenario::DODGING_TARGET) ?
        -30.f :
        (actor->currentScenario == InvaderBehaviorComponent::Scenario::SLIDING_BOTH ||
         actor->currentScenario == InvaderBehaviorComponent::Scenario::WAGGING_BOTH) ?
        30.f :
        0.f;
        offsetY = 30.f;
        break;
    }
    case InvaderBehaviorComponent::Role::OBSERVER:
    {
        offsetX = -50.f;
        offsetY = -50.f;
        break;
    }
    default:
    {
        DVASSERT(false);
        break;
    }
    }

    Vector2 pos(offsetX, offsetY);
    Vector2 normalizedPos = InvaderMovingSystemDetail::GetNormalizedTeleportPosition(pos);
    NetworkReplicationComponent* replComp = actor->GetEntity()->GetComponent<NetworkReplicationComponent>();
    DVASSERT(replComp);
    NetworkPlayerID playerID = replComp->GetNetworkPlayerID();
    Logger::Debug("[InvaderBehavior] Initializing %s invader position to (%.1f, %.1f)",
                  InvaderBehaviorComponent::roleNames[actor->role].c_str(), offsetX, offsetY);
    GetScene()->GetSingleComponent<ActionsSingleComponent>()->AddAnalogAction(FastName("TELEPORT"), normalizedPos, playerID);
}

BotTaskComponent* InvaderBehaviorSystem::CreateInitialTask(InvaderBehaviorComponent* actor)
{
    using namespace InvaderBehaviorSystemDetails;
    return new WaitTaskComponent(WaitTaskComponent::Type::DELAY, DELAY_DURATION);
}

BotTaskComponent* InvaderBehaviorSystem::CreateNextTask(InvaderBehaviorComponent* actor)
{
    InvaderBehaviorComponent* target = nullptr;
    if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
    {
        for (InvaderBehaviorComponent* other : agents)
        {
            if (other != actor && other->role == InvaderBehaviorComponent::Role::TARGET)
            {
                HealthComponent* health = other->GetEntity()->GetComponent<HealthComponent>();
                if (!health || health->GetHealth() == 0)
                {
                    continue;
                }

                target = other;
                break;
            }
        }

        if (!target)
        {
            Logger::Debug("[InvaderBehavior] Failed to find a task for %s invader: no target found",
                          InvaderBehaviorComponent::roleNames[actor->role].c_str());
            return nullptr;
        }
    }

    switch (actor->currentScenario)
    {
    case InvaderBehaviorComponent::Scenario::STILL:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            return new ShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::SLIDING_TARGET:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            return new ShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
        }
        else if (actor->role == InvaderBehaviorComponent::Role::TARGET)
        {
            return new SlideToBorderTaskComponent();
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::SLIDING_SHOOTER:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            ShootIfSeeingTargetTaskComponent* shootTask = new ShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
            SlideToBorderTaskComponent* slideTask = new SlideToBorderTaskComponent();
            return new CompositeTaskComponent(CompositeTaskComponent::Type::AND, slideTask, shootTask);
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::SLIDING_BOTH:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            ShootIfSeeingTargetTaskComponent* shootTask = new ShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
            SlideToBorderTaskComponent* slideTask = new SlideToBorderTaskComponent(true);
            return new CompositeTaskComponent(CompositeTaskComponent::Type::AND, slideTask, shootTask);
        }
        else if (actor->role == InvaderBehaviorComponent::Role::TARGET)
        {
            return new SlideToBorderTaskComponent(false);
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::WAGGING_TARGET:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            return new ShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
        }
        else if (actor->role == InvaderBehaviorComponent::Role::TARGET)
        {
            return new WagToBorderTaskComponent();
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::WAGGING_SHOOTER:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            ShootIfSeeingTargetTaskComponent* shootTask = new ShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
            WagToBorderTaskComponent* wagTask = new WagToBorderTaskComponent();
            return new CompositeTaskComponent(CompositeTaskComponent::Type::AND, wagTask, shootTask);
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::WAGGING_BOTH:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            ShootIfSeeingTargetTaskComponent* shootTask = new ShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
            WagToBorderTaskComponent* wagTask = new WagToBorderTaskComponent(true);
            return new CompositeTaskComponent(CompositeTaskComponent::Type::AND, wagTask, shootTask);
        }
        else if (actor->role == InvaderBehaviorComponent::Role::TARGET)
        {
            return new WagToBorderTaskComponent(false);
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::DODGING_TARGET:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            return new ShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
        }
        else if (actor->role == InvaderBehaviorComponent::Role::TARGET)
        {
            return new DodgeCenterTaskComponent();
        }
        break;
    }
    default:
        DVASSERT(false);
    }

    // Observers and others having not received any task are just waiting, forever.
    return new WaitTaskComponent(WaitTaskComponent::Type::DELAY, FLOAT_MAX);
}

void InvaderBehaviorSystem::ClearCurrentTask(InvaderBehaviorComponent* agent)
{
    if (agent->currentTask)
    {
        agent->currentTask->TraverseTaskTree([](BotTaskComponent* task) { task->GetEntity()->RemoveComponent(task); });
        agent->currentTask = nullptr;
        Logger::Debug("[InvaderBehavior] Task cleared for %s invader",
                      InvaderBehaviorComponent::roleNames[agent->role].c_str());
    }
}

void InvaderBehaviorSystem::SetCurrentTask(InvaderBehaviorComponent* agent, BotTaskComponent* task)
{
    ClearCurrentTask(agent);
    if (task)
    {
        Entity* entity = agent->GetEntity();
        task->TraverseTaskTree([&entity](BotTaskComponent* task) { entity->AddComponent(task); });
        agent->currentTask = task;
        Logger::Debug("[InvaderBehavior] Task %s given to %s invader",
                      ReflectedObject(task).GetReflectedType()->GetPermanentName().c_str(),
                      InvaderBehaviorComponent::roleNames[agent->role].c_str());
    }
}
