#include "ShooterBehaviorSystem.h"
#include "Components/AI/MoveToPointTaskComponent.h"
#include "Components/AI/AttackTaskComponent.h"
#include "Components/AI/WaitTaskComponent.h"
#include "Components/AI/CompositeTaskComponent.h"
#include "Components/HealthComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "Systems/GameInputSystem.h"

#include <Reflection/ReflectionRegistrator.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>

namespace ShooterBehaviorSystemDetail
{
static const Vector2 CENTER(0.f, 0.f);
}

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterBehaviorSystem)
{
    ReflectionRegistrator<ShooterBehaviorSystem>::Begin()[M::Tags("bot", "shooterbot")]
    .ConstructorByPointer<Scene*>() // TODO: system out of place. Should be in gameplay group, or be a part of fw.
    .Method("ProcessFixed", &ShooterBehaviorSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 4.1f)]
    .End();
}

ShooterBehaviorSystem::ShooterBehaviorSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<ShooterBehaviorComponent>() | ComponentUtils::MakeMask<NetworkReplicationComponent>() | ComponentUtils::MakeMask<NetworkInputComponent>())
{
    otherAgents = scene->AquireComponentGroup<ShooterBehaviorComponent, ShooterBehaviorComponent>();
}

void ShooterBehaviorSystem::ProcessFixed(float32 timeElapsed)
{
    ProcessPendingAgents();

    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    if (!netTimeComp->IsInitialized())
    {
        return;
    }

    for (ShooterBehaviorComponent* agent : agents)
    {
        BotTaskComponent* currentTask = agent->currentTask;
        if (!currentTask)
        {
            SetCurrentTask(agent, CreateInititalTask(agent));
        }
        else if (currentTask->GetStatus() != BotTaskStatus::IN_PROGRESS)
        {
            SetCurrentTask(agent, CreateNextTask(agent));
        }
    }
}

void ShooterBehaviorSystem::AddEntity(Entity* entity)
{
    ShooterBehaviorComponent* agent = entity->GetComponent<ShooterBehaviorComponent>();
    DVASSERT(agent);
    pendingAgents.push_back(agent);
}

void ShooterBehaviorSystem::ProcessPendingAgents()
{
    for (ShooterBehaviorComponent* agent : pendingAgents)
    {
        // TODO: current solution don't support multiple actors
        DVASSERT(agent->isActor);
        BattleOptionsSingleComponent* optionsComp = GetScene()->GetSingletonComponent<BattleOptionsSingleComponent>();
        uint32 seed = static_cast<uint32>(std::hash<std::string>{}(optionsComp->options.token.c_str()));
        localRandom.Seed(seed);

        agents.push_back(agent);
    }

    pendingAgents.clear();
}

void ShooterBehaviorSystem::RemoveEntity(Entity* entity)
{
    ShooterBehaviorComponent* agent = entity->GetComponent<ShooterBehaviorComponent>();
    DVASSERT(agent);

    auto found = std::remove(agents.begin(), agents.end(), agent);
    if (found != agents.end())
    {
        ClearCurrentTask(agent);
        agents.erase(found, agents.end());
    }

    auto foundPending = std::remove(pendingAgents.begin(), pendingAgents.end(), agent);
    pendingAgents.erase(found, pendingAgents.end());
}

BotTaskComponent* ShooterBehaviorSystem::CreateInititalTask(ShooterBehaviorComponent* actor)
{
    float rad = localRandom.RandFloat32InBounds(800.0f, 1800.f);
    float angle = localRandom.RandFloat32InBounds(-PI, PI);
    Vector2 pos(rad * cosf(angle), rad * sinf(angle));
    Vector2 normalizedPos = GameInputSystemDetail::GetNormalizedTeleportPosition(pos);
    NetworkReplicationComponent* replComp = actor->GetEntity()->GetComponent<NetworkReplicationComponent>();
    NetworkPlayerID playerID = replComp->GetNetworkPlayerID();
    GetScene()->GetSingletonComponent<ActionsSingleComponent>()->AddAnalogAction(FastName("TELEPORT"), normalizedPos, playerID);

    float waitPeriod = localRandom.RandFloat32InBounds(40.f, 60.f);
    WaitTaskComponent* waitTask = new WaitTaskComponent(WaitTaskComponent::Type::DELAY, waitPeriod);

    return waitTask;
}

BotTaskComponent* ShooterBehaviorSystem::CreateNextTask(ShooterBehaviorComponent* actor)
{
    Vector3 curPos3 = actor->GetEntity()->GetWorldTransform().GetTranslationVector();
    Vector2 curPos(curPos3.x, curPos3.y);

    Vector2 delta = curPos - ShooterBehaviorSystemDetail::CENTER;
    float curDist = delta.Length();

    BotTaskComponent* task = nullptr;
    if ((localRandom.RandFloat() < 0.6f))
    {
        task = CreateNextMoveTask(actor);
    }

    if (!task)
    {
        task = CreateNextAttackTask(actor);
    }

    if (!task)
    {
        task = new WaitTaskComponent(WaitTaskComponent::Type::DELAY, 10.f);
    }

    return task;
}

BotTaskComponent* ShooterBehaviorSystem::CreateNextMoveTask(ShooterBehaviorComponent* actor)
{
    Entity* entity = actor->GetEntity();

    Vector3 curPos3 = entity->GetWorldTransform().GetTranslationVector();
    Vector2 curPos(curPos3.x, curPos3.y);
    Vector2 delta = curPos - ShooterBehaviorSystemDetail::CENTER;
    float curDist = delta.Length();

    if (curDist < 40.f)
    {
        return nullptr;
    }

    float newDist = std::max(30.f, curDist - localRandom.RandFloat32InBounds(20.f, 40.f));
    float boundAngle = 0.7f * acosf(newDist / curDist);
    float angle = localRandom.RandFloat32InBounds(-boundAngle, boundAngle);

    Vector2 ort(delta.y, -delta.x);

    Vector2 targetPoint = ShooterBehaviorSystemDetail::CENTER + (newDist / curDist) * (delta * cosf(angle) + ort * sinf(angle));
    Vector3 targetPoint3(targetPoint.x, targetPoint.y, 0.f);

    MoveToPointTaskComponent* moveTask = new MoveToPointTaskComponent(targetPoint3, 0.5f);
    WaitTaskComponent* waitTask = new WaitTaskComponent(WaitTaskComponent::Type::DELAY, 15.f);
    CompositeTaskComponent* res = new CompositeTaskComponent(CompositeTaskComponent::Type::OR, moveTask, waitTask);

    return res;
}

BotTaskComponent* ShooterBehaviorSystem::CreateNextAttackTask(ShooterBehaviorComponent* actor)
{
    Vector3 curPos3 = actor->GetEntity()->GetWorldTransform().GetTranslationVector();

    float closestSqrDist = FLOAT_MAX;
    ShooterBehaviorComponent* closestTarget = nullptr;
    for (ShooterBehaviorComponent* otherAgent : otherAgents->components)
    {
        if (otherAgent == actor)
        {
            continue;
        }

        Vector3 otherPos3 = otherAgent->GetEntity()->GetWorldTransform().GetTranslationVector();
        float sqrDist = (curPos3 - otherPos3).SquareLength();
        if (sqrDist < closestSqrDist)
        {
            HealthComponent* health = otherAgent->GetEntity()->GetComponent<HealthComponent>();
            if (!health || (health->GetHealth() == 0))
            {
                continue;
            }

            closestSqrDist = sqrDist;
            closestTarget = otherAgent;
        }
    }

    if (!closestTarget)
    {
        return nullptr;
    }

    AttackTaskComponent* attackTask = new AttackTaskComponent(closestTarget->GetEntity()->GetID(), 2.f, 2.f);
    float attackPeriod = localRandom.RandFloat32InBounds(10.f, 12.f);
    WaitTaskComponent* waitTask = new WaitTaskComponent(WaitTaskComponent::Type::DELAY, attackPeriod);
    CompositeTaskComponent* res = new CompositeTaskComponent(CompositeTaskComponent::Type::OR, attackTask, waitTask);

    return res;
}

void ShooterBehaviorSystem::ClearCurrentTask(ShooterBehaviorComponent* agent)
{
    if (agent->currentTask)
    {
        agent->currentTask->TraverseTaskTree([](BotTaskComponent* task) { task->GetEntity()->RemoveComponent(task); });
        agent->currentTask = nullptr;
    }
}

void ShooterBehaviorSystem::SetCurrentTask(ShooterBehaviorComponent* agent, BotTaskComponent* task)
{
    ClearCurrentTask(agent);
    if (task)
    {
        Entity* entity = agent->GetEntity();
        task->TraverseTaskTree([&entity](BotTaskComponent* task) { entity->AddComponent(task); });
        agent->currentTask = task;
    }
}
