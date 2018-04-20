#include "InvaderBehaviorSystem.h"

#include "Components/HealthComponent.h"
#include "CommonTaskComponents.h"
#include "InvaderTaskComponents.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "Scene3D/Components/SingleComponents/ActionsSingleComponent.h"
#include "Systems/InvaderMovingSystem.h"

#include <Logger/Logger.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedObject.h>

using namespace DAVA;

namespace InvaderBehaviorSystemDetails
{
static const float32 DELAY_DURATION = 0.5f;
static const float32 SCENARIO_DURATION = 2.5f;
}

DAVA_VIRTUAL_REFLECTION_IMPL(InvaderBehaviorSystem)
{
    ReflectionRegistrator<InvaderBehaviorSystem>::Begin()[M::Tags("bot", "invaderbot")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &BehaviorSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 15.1f)]
    .End();
}

InvaderBehaviorSystem::InvaderBehaviorSystem(Scene* scene)
    : BehaviorSystem(scene, ComponentMask(),
                     scene->AquireComponentGroupWithMatcher<AnyOfEntityMatcher, BaseOfTypeMatcher, BehaviorComponent, InvaderBehaviorComponent>())
{
}

bool InvaderBehaviorSystem::CheckStartConditions(float32 timeElapsed)
{
    // wait till all roles are present
    if (!allRolesPresent && pendingAgents->components.size() < InvaderBehaviorComponent::ROLES_COUNT)
    {
        return false;
    }

    allRolesPresent = true;

    // sort to give roles in same order on all clients
    std::sort(pendingAgents->components.begin(), pendingAgents->components.end(),
              [](const BehaviorComponent* lhs, const BehaviorComponent* rhs)
              {
                  NetworkReplicationComponent* lhsReplComp = lhs->GetEntity()->GetComponent<NetworkReplicationComponent>();
                  NetworkReplicationComponent* rhsReplComp = rhs->GetEntity()->GetComponent<NetworkReplicationComponent>();
                  return lhsReplComp->GetNetworkPlayerID() < rhsReplComp->GetNetworkPlayerID();
              });

    return true;
}

void InvaderBehaviorSystem::ProcessPending(BehaviorComponent* pendingAgent)
{
    using namespace InvaderBehaviorSystemDetails;

    InvaderBehaviorComponent* agent = static_cast<InvaderBehaviorComponent*>(pendingAgent);
    agent->role = static_cast<InvaderBehaviorComponent::Role>(
    nextRole < InvaderBehaviorComponent::ROLES_COUNT ? nextRole : InvaderBehaviorComponent::OBSERVER);
    nextRole++;
    agent->timeBeforeScenarioSwitch = SCENARIO_DURATION;

    Logger::Debug("[InvaderBehaviorSystem] Setting role %s for invader id %d",
                  InvaderBehaviorComponent::roleNames[agent->role].c_str(),
                  agent->GetEntity()->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID());
}

bool InvaderBehaviorSystem::UpdateScenario(BehaviorComponent* agent, float32 timeElapsed)
{
    using namespace InvaderBehaviorSystemDetails;

    InvaderBehaviorComponent* actor = static_cast<InvaderBehaviorComponent*>(agent);

    actor->timeBeforeScenarioSwitch -= timeElapsed;
    if (actor->timeBeforeScenarioSwitch <= 0.f)
    {
        actor->syncCounter++;

        actor->currentScenario++;
        actor->currentScenario %= InvaderBehaviorComponent::SCENARIOS_COUNT;
        actor->timeBeforeScenarioSwitch = SCENARIO_DURATION;
        Logger::Debug("[InvaderBehaviorSystem] Switching scenario to %s",
                      InvaderBehaviorComponent::scenarioNames[actor->currentScenario].c_str());
        return true;
    }

    return false;
}

void InvaderBehaviorSystem::InitPosition(BehaviorComponent* agent)
{
    InvaderBehaviorComponent* actor = static_cast<InvaderBehaviorComponent*>(agent);

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
    NormalizeAnalog(pos, InvaderMovingSystemDetail::TELEPORT_HALF_RANGE);
    NetworkReplicationComponent* replComp = actor->GetEntity()->GetComponent<NetworkReplicationComponent>();
    DVASSERT(replComp);
    NetworkPlayerID playerID = replComp->GetNetworkPlayerID();
    Logger::Debug("[InvaderBehavior] Initializing %s invader position to (%.1f, %.1f)",
                  InvaderBehaviorComponent::roleNames[actor->role].c_str(), offsetX, offsetY);
    GetScene()->GetSingleComponentForWrite<ActionsSingleComponent>(this)->AddAnalogAction(FastName("TELEPORT"), pos, playerID);
}

BotTaskComponent* InvaderBehaviorSystem::CreateInitialTask(BehaviorComponent* agent)
{
    using namespace InvaderBehaviorSystemDetails;
    return new WaitTaskComponent(WaitTaskComponent::Type::DELAY, DELAY_DURATION);
}

BotTaskComponent* InvaderBehaviorSystem::CreateNextTask(BehaviorComponent* agent)
{
    InvaderBehaviorComponent* actor = static_cast<InvaderBehaviorComponent*>(agent);
    InvaderBehaviorComponent* target = nullptr;
    if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
    {
        for (BehaviorComponent* otherAgent : agents->components)
        {
            InvaderBehaviorComponent* other = static_cast<InvaderBehaviorComponent*>(otherAgent);
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
            return new InvaderShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::SLIDING_TARGET:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            return new InvaderShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
        }
        else if (actor->role == InvaderBehaviorComponent::Role::TARGET)
        {
            return new InvaderSlideToBorderTaskComponent();
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::SLIDING_SHOOTER:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            InvaderShootIfSeeingTargetTaskComponent* shootTask = new InvaderShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
            InvaderSlideToBorderTaskComponent* slideTask = new InvaderSlideToBorderTaskComponent();
            return new CompositeTaskComponent(CompositeTaskComponent::Type::AND, slideTask, shootTask);
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::SLIDING_BOTH:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            InvaderShootIfSeeingTargetTaskComponent* shootTask = new InvaderShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
            InvaderSlideToBorderTaskComponent* slideTask = new InvaderSlideToBorderTaskComponent(true);
            return new CompositeTaskComponent(CompositeTaskComponent::Type::AND, slideTask, shootTask);
        }
        else if (actor->role == InvaderBehaviorComponent::Role::TARGET)
        {
            return new InvaderSlideToBorderTaskComponent(false);
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::WAGGING_TARGET:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            return new InvaderShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
        }
        else if (actor->role == InvaderBehaviorComponent::Role::TARGET)
        {
            return new InvaderWagToBorderTaskComponent();
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::WAGGING_SHOOTER:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            InvaderShootIfSeeingTargetTaskComponent* shootTask = new InvaderShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
            InvaderWagToBorderTaskComponent* wagTask = new InvaderWagToBorderTaskComponent();
            return new CompositeTaskComponent(CompositeTaskComponent::Type::AND, wagTask, shootTask);
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::WAGGING_BOTH:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            InvaderShootIfSeeingTargetTaskComponent* shootTask = new InvaderShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
            InvaderWagToBorderTaskComponent* wagTask = new InvaderWagToBorderTaskComponent(true);
            return new CompositeTaskComponent(CompositeTaskComponent::Type::AND, wagTask, shootTask);
        }
        else if (actor->role == InvaderBehaviorComponent::Role::TARGET)
        {
            return new InvaderWagToBorderTaskComponent(false);
        }
        break;
    }
    case InvaderBehaviorComponent::Scenario::DODGING_TARGET:
    {
        if (actor->role == InvaderBehaviorComponent::Role::SHOOTER)
        {
            return new InvaderShootIfSeeingTargetTaskComponent(target->GetEntity()->GetID());
        }
        else if (actor->role == InvaderBehaviorComponent::Role::TARGET)
        {
            return new InvaderDodgeCenterTaskComponent();
        }
        break;
    }
    default:
        DVASSERT(false);
    }

    // Observers and others having not received any task are just waiting, forever.
    return new WaitTaskComponent(WaitTaskComponent::Type::DELAY, FLOAT_MAX);
}
