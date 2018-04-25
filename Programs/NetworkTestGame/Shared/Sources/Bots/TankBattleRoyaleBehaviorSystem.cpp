#include "TankBattleRoyaleBehaviorSystem.h"
#include "CommonTaskComponents.h"
#include "TankTaskComponents.h"
#include "Components/HealthComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "Systems/GameInputSystem.h"

#include <Reflection/ReflectionRegistrator.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>

using namespace DAVA;

namespace TankBattleRoyaleBehaviorSystemDetail
{
static const Vector2 CENTER(0.f, 0.f);
}

DAVA_VIRTUAL_REFLECTION_IMPL(TankBattleRoyaleBehaviorSystem)
{
    ReflectionRegistrator<TankBattleRoyaleBehaviorSystem>::Begin()[M::SystemTags("bot", "tankbot")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &BehaviorSystem::ProcessFixed)[M::SystemProcessInfo(SPI::Group::EngineBegin, SPI::Type::Fixed, 15.0f)]
    .End();
}

TankBattleRoyaleBehaviorSystem::TankBattleRoyaleBehaviorSystem(Scene* scene)
    : BehaviorSystem(scene, ComponentMask(),
                     scene->AquireComponentGroupWithMatcher<AnyOfEntityMatcher, BaseOfTypeMatcher, BehaviorComponent, BattleRoyaleBehaviorComponent>())
{
}

void TankBattleRoyaleBehaviorSystem::ProcessPending(BehaviorComponent* pendingAgent)
{
    if (pendingAgent->isActor)
    {
        BattleRoyaleBehaviorComponent* agent = static_cast<BattleRoyaleBehaviorComponent*>(pendingAgent);
        const BattleOptionsSingleComponent* optionsComp = GetScene()->GetSingleComponentForRead<BattleOptionsSingleComponent>(this);
        uint32 seed = static_cast<uint32>(std::hash<std::string>{}(optionsComp->options.token.c_str()));
        agent->localRandom.Seed(seed);
    }
}

void TankBattleRoyaleBehaviorSystem::InitPosition(BehaviorComponent* agent)
{
    BattleRoyaleBehaviorComponent* actor = static_cast<BattleRoyaleBehaviorComponent*>(agent);

    float rad = actor->localRandom.RandFloat32InBounds(800.0f, 1800.f);
    float angle = actor->localRandom.RandFloat32InBounds(-PI, PI);
    Vector2 pos(rad * cosf(angle), rad * sinf(angle));
    NormalizeAnalog(pos, GameInputSystemDetail::TELEPORT_HALF_RANGE);
    NetworkReplicationComponent* replComp = actor->GetEntity()->GetComponent<NetworkReplicationComponent>();
    NetworkPlayerID playerID = replComp->GetNetworkPlayerID();
    GetScene()->GetSingleComponentForWrite<ActionsSingleComponent>(this)->AddAnalogAction(FastName("TELEPORT"), pos, playerID);
}

BotTaskComponent* TankBattleRoyaleBehaviorSystem::CreateInitialTask(BehaviorComponent* agent)
{
    BattleRoyaleBehaviorComponent* actor = static_cast<BattleRoyaleBehaviorComponent*>(agent);
    float waitPeriod = actor->localRandom.RandFloat32InBounds(40.f, 60.f);
    return new WaitTaskComponent(WaitTaskComponent::Type::DELAY, waitPeriod);
}

BotTaskComponent* TankBattleRoyaleBehaviorSystem::CreateNextTask(BehaviorComponent* agent)
{
    BattleRoyaleBehaviorComponent* actor = static_cast<BattleRoyaleBehaviorComponent*>(agent);

    BotTaskComponent* task = nullptr;
    if ((actor->localRandom.RandFloat() < 0.6f))
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

BotTaskComponent* TankBattleRoyaleBehaviorSystem::CreateNextMoveTask(BattleRoyaleBehaviorComponent* actor)
{
    Entity* entity = actor->GetEntity();

    Vector3 curPos3 = entity->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();
    Vector2 curPos(curPos3.x, curPos3.y);
    Vector2 delta = curPos - TankBattleRoyaleBehaviorSystemDetail::CENTER;
    float curDist = delta.Length();

    if (curDist < 40.f)
    {
        return nullptr;
    }

    float newDist = std::max(30.f, curDist - actor->localRandom.RandFloat32InBounds(20.f, 40.f));
    float boundAngle = 0.7f * acosf(newDist / curDist);
    float angle = actor->localRandom.RandFloat32InBounds(-boundAngle, boundAngle);

    Vector2 ort(delta.y, -delta.x);

    Vector2 targetPoint = TankBattleRoyaleBehaviorSystemDetail::CENTER + (newDist / curDist) * (delta * cosf(angle) + ort * sinf(angle));
    Vector3 targetPoint3(targetPoint.x, targetPoint.y, 0.f);

    TankMoveToPointTaskComponent* moveTask = new TankMoveToPointTaskComponent(targetPoint3, 0.5f);
    WaitTaskComponent* waitTask = new WaitTaskComponent(WaitTaskComponent::Type::DELAY, 15.f);
    CompositeTaskComponent* res = new CompositeTaskComponent(CompositeTaskComponent::Type::OR, moveTask, waitTask);

    return res;
}

BotTaskComponent* TankBattleRoyaleBehaviorSystem::CreateNextAttackTask(BattleRoyaleBehaviorComponent* actor)
{
    Vector3 curPos3 = actor->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();

    float closestSqrDist = FLOAT_MAX;
    BattleRoyaleBehaviorComponent* closestTarget = nullptr;
    for (BehaviorComponent* agent : agents->components)
    {
        BattleRoyaleBehaviorComponent* other = static_cast<BattleRoyaleBehaviorComponent*>(agent);
        if (other == actor)
        {
            continue;
        }

        Vector3 otherPos3 = other->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();
        float sqrDist = (curPos3 - otherPos3).SquareLength();
        if (sqrDist < closestSqrDist)
        {
            HealthComponent* health = other->GetEntity()->GetComponent<HealthComponent>();
            if (!health || (health->GetHealth() == 0))
            {
                continue;
            }

            closestSqrDist = sqrDist;
            closestTarget = other;
        }
    }

    if (!closestTarget)
    {
        return nullptr;
    }

    TankAttackTaskComponent* attackTask = new TankAttackTaskComponent(closestTarget->GetEntity()->GetID(), 2.f, 2.f);
    float attackPeriod = actor->localRandom.RandFloat32InBounds(10.f, 12.f);
    WaitTaskComponent* waitTask = new WaitTaskComponent(WaitTaskComponent::Type::DELAY, attackPeriod);
    CompositeTaskComponent* res = new CompositeTaskComponent(CompositeTaskComponent::Type::OR, attackTask, waitTask);

    return res;
}
