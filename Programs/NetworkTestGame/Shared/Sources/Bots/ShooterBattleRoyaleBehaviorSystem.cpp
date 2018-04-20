#include "ShooterBattleRoyaleBehaviorSystem.h"
#include "CommonTaskComponents.h"
#include "ShooterTaskComponents.h"
#include "ShooterUtils.h"
#include "Components/HealthComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "Systems/ShooterPlayerMovementSystem.h"

#include <Reflection/ReflectionRegistrator.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>

using namespace DAVA;

namespace ShooterBattleRoyaleBehaviorSystemDetail
{
static const float32 WAIT_TO_START = 10.f;
static const Vector2 CENTER(0.f, 0.f);
}

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterBattleRoyaleBehaviorSystem)
{
    ReflectionRegistrator<ShooterBattleRoyaleBehaviorSystem>::Begin()[M::Tags("bot", "shooterbot")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &BehaviorSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 15.2f)]
    .End();
}

ShooterBattleRoyaleBehaviorSystem::ShooterBattleRoyaleBehaviorSystem(Scene* scene)
    : BehaviorSystem(scene, ComponentMask(),
                     scene->AquireComponentGroupWithMatcher<AnyOfEntityMatcher, BaseOfTypeMatcher, BehaviorComponent, BattleRoyaleBehaviorComponent>())
{
    roles = scene->AquireComponentGroup<ShooterRoleComponent>();
}

bool ShooterBattleRoyaleBehaviorSystem::CheckStartConditions(float32 timeElapsed)
{
    waitTime += timeElapsed;
    return waitTime > ShooterBattleRoyaleBehaviorSystemDetail::WAIT_TO_START;
}

void ShooterBattleRoyaleBehaviorSystem::ProcessPending(BehaviorComponent* pendingAgent)
{
    if (pendingAgent->isActor)
    {
        BattleRoyaleBehaviorComponent* agent = static_cast<BattleRoyaleBehaviorComponent*>(pendingAgent);
        const BattleOptionsSingleComponent* optionsComp = GetScene()->GetSingleComponentForRead<BattleOptionsSingleComponent>(this);
        uint32 seed = static_cast<uint32>(std::hash<std::string>{}(optionsComp->options.token.c_str()));
        agent->localRandom.Seed(seed);
    }
}

void ShooterBattleRoyaleBehaviorSystem::InitPosition(BehaviorComponent* agent)
{
    BattleRoyaleBehaviorComponent* actor = static_cast<BattleRoyaleBehaviorComponent*>(agent);

    NetworkReplicationComponent* replComp = actor->GetEntity()->GetComponent<NetworkReplicationComponent>();
    NetworkPlayerID playerId = replComp->GetNetworkPlayerID();

    float rad = actor->localRandom.RandFloat32InBounds(150.0f, 250.f);
    float angle = actor->localRandom.RandFloat32InBounds(-PI, PI);
    Vector2 pos(rad * cosf(angle), rad * sinf(angle));
    pos += ShooterBattleRoyaleBehaviorSystemDetail::CENTER;
    NormalizeAnalog(pos, ShooterMovementSystemDetail::TELEPORT_HALF_RANGE);
    GetScene()->GetSingleComponentForWrite<ActionsSingleComponent>(this)->AddAnalogAction(SHOOTER_ACTION_TELEPORT, pos, playerId);
}

BotTaskComponent* ShooterBattleRoyaleBehaviorSystem::CreateInitialTask(BehaviorComponent* agent)
{
    BattleRoyaleBehaviorComponent* actor = static_cast<BattleRoyaleBehaviorComponent*>(agent);
    float waitPeriod = actor->localRandom.RandFloat32InBounds(10.f, 12.f);
    return new WaitTaskComponent(WaitTaskComponent::Type::DELAY, waitPeriod);
}

BotTaskComponent* ShooterBattleRoyaleBehaviorSystem::CreateNextTask(BehaviorComponent* agent)
{
    BattleRoyaleBehaviorComponent* actor = static_cast<BattleRoyaleBehaviorComponent*>(agent);

    BotTaskComponent* task = nullptr;
    if (actor->localRandom.RandFloat() < actor->lifetime / 180.f) // increase attack chance with time
    {
        task = CreateNextAttackTask(actor);
    }

    if (!task)
    {
        task = CreateNextMoveTask(actor);
    }

    if (!task)
    {
        task = new WaitTaskComponent(WaitTaskComponent::Type::DELAY, 10.f);
    }

    return task;
}

BotTaskComponent* ShooterBattleRoyaleBehaviorSystem::CreateNextAttackTask(BattleRoyaleBehaviorComponent* actor)
{
    if (!CanAct(actor->GetEntity()))
    {
        return nullptr;
    }

    Vector3 actorPos = actor->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();

    float32 minSqrDist = FLOAT_MAX;
    BattleRoyaleBehaviorComponent* target = nullptr;
    for (BehaviorComponent* otherAgent : agents->components)
    {
        BattleRoyaleBehaviorComponent* other = static_cast<BattleRoyaleBehaviorComponent*>(otherAgent);
        if (other == actor)
        {
            continue;
        }

        Vector3 otherPos = other->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();
        float32 sqrDist = (otherPos - actorPos).SquareLength();
        if (sqrDist < minSqrDist)
        {
            HealthComponent* health = other->GetEntity()->GetComponent<HealthComponent>();
            if (!health || health->GetHealth() == 0)
            {
                continue;
            }

            minSqrDist = sqrDist;
            target = other;
        }
    }

    if (target == nullptr || actor->localRandom.RandFloat() < Clamp((minSqrDist - 100.f) / 1500.f, 0.f, 1.f))
    {
        return nullptr;
    }

    float64 randFloat = actor->localRandom.RandFloat();

    BotTaskComponent* attackTask = nullptr;
    float32 attackPeriod;
    if (randFloat < 0.4f)
    {
        attackTask = new ShooterAttackCirclingAroundTaskComponent(target->GetEntity()->GetID(), randFloat < 0.2);
        attackPeriod = 5.f;
    }
    else if (randFloat < 0.7f)
    {
        attackTask = new ShooterAttackPursuingTargetTaskComponent(target->GetEntity()->GetID());
        attackPeriod = 10.f;
    }
    else if (randFloat < 0.9f)
    {
        attackTask = new ShooterAttackStandingStillTaskComponent(target->GetEntity()->GetID());
        attackPeriod = 5.f;
    }
    else
    {
        attackTask = new ShooterAttackWaggingTaskComponent(target->GetEntity()->GetID());
        attackPeriod = 7.f;
    }

    WaitTaskComponent* waitTask = new WaitTaskComponent(WaitTaskComponent::Type::DELAY, attackPeriod);
    return new CompositeTaskComponent(CompositeTaskComponent::Type::OR, attackTask, waitTask);
}

BotTaskComponent* ShooterBattleRoyaleBehaviorSystem::CreateNextMoveTask(BattleRoyaleBehaviorComponent* actor)
{
    if (!CanAct(actor->GetEntity()))
    {
        return nullptr;
    }

    Vector3 actorPos = actor->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();
    Vector3 destination = Vector3::Zero;
    static bool usedCar = false;

    // ride a car
    float32 minDistToCar = FLOAT_MAX;
    ShooterRoleComponent* car = nullptr;
    if (!usedCar)
    {
        for (ShooterRoleComponent* role : roles->components)
        {
            if (role->GetRole() != ShooterRoleComponent::Role::Car)
            {
                continue;
            }

            Vector3 pos = role->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();
            float32 dist = (pos - actorPos).Length();

            if (dist < minDistToCar)
            {
                minDistToCar = dist;
                car = role;
            }
        }
    }

    if (car)
    {
        if (minDistToCar <= SHOOTER_CAR_MAX_INTERACT_DISTANCE)
        {
            usedCar = true;
            Vector3 point(ShooterBattleRoyaleBehaviorSystemDetail::CENTER.x, ShooterBattleRoyaleBehaviorSystemDetail::CENTER.y, 0.f);
            return new ShooterDriveTaskComponent(point, car->GetEntity()->GetID());
        }
        else if (minDistToCar < 100.f)
        {
            destination = car->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();
        }
    }

    // go by foot
    if (destination.IsZero())
    {
        Vector2 curPos(actorPos.x, actorPos.y);
        Vector2 delta = curPos - ShooterBattleRoyaleBehaviorSystemDetail::CENTER;
        float curDist = delta.Length();
        if (curDist < 10.f)
        {
            return nullptr;
        }

        float32 newDist = std::max(10.f, curDist - actor->localRandom.RandFloat32InBounds(20.f, 40.f));
        float32 angle = actor->localRandom.RandFloat32InBounds(-PI, PI);
        Vector2 newPos(newDist * cosf(angle), newDist * sinf(angle));
        newPos += ShooterBattleRoyaleBehaviorSystemDetail::CENTER;
        destination = Vector3(newPos.x, newPos.y, 0);
    }

    float64 randFloat = actor->localRandom.RandFloat();

    BotTaskComponent* moveTask = nullptr;
    float32 movePeriod;
    if (randFloat < 0.4f)
    {
        moveTask = new ShooterMoveToPointWindingTaskComponent(destination);
        movePeriod = 12.f;
    }
    else if (randFloat < 0.7f)
    {
        moveTask = new ShooterHangAroundTaskComponent(destination, actor->localRandom.RandFloat32InBounds(-PI, PI));
        movePeriod = 15.f;
    }
    else
    {
        moveTask = new ShooterMoveToPointShortestTaskComponent(destination);
        movePeriod = 9.f;
    }

    WaitTaskComponent* waitTask = new WaitTaskComponent(WaitTaskComponent::Type::DELAY, movePeriod);
    return new CompositeTaskComponent(CompositeTaskComponent::Type::OR, moveTask, waitTask);
}
