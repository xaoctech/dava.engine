#include "BotTaskSystem.h"
#include "Components/HealthComponent.h"
#include "Components/ShootComponent.h"

#include <Math/Matrix2.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>
#include <Utils/Random.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(BotTaskSystem)
{
    ReflectionRegistrator<BotTaskSystem>::Begin()[M::Tags("bot", "shooterbot")]
    .ConstructorByPointer<Scene*>() // TODO: system out of place. Should be in gameplay group, or be a part of fw.
    .Method("ProcessFixed", &BotTaskSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 4.2f)]
    .End();
}

namespace
{
// helper
bool ComputeShotYawWithDeflection(
float& outAngle, float& outDist,
const Vector2& shooterPos, float bulletSpeed,
const Vector2& targetPos, const Vector2& targetVelocity)
{
    DVASSERT(bulletSpeed > 1e-8f);

    Vector2 delta = targetPos - shooterPos;
    float dist = delta.Length();
    if (dist < 1e-8f)
    {
        return false;
    }

    // t, angle
    Vector2 cur(dist / bulletSpeed, atan2f(delta.y, delta.x));
    Vector2 prev;
    Vector2 f;
    Matrix2 jacobian;
    Matrix2 invJacobian;

    // Newton iteration
    for (int iter = 0; iter < 3; ++iter)
    {
        prev = cur;

        float t = prev.x;
        float sinA = sinf(prev.y);
        float cosA = cosf(prev.y);

        f.x = -cosA * t * bulletSpeed + t * targetVelocity.x + delta.x;
        f.y = -sinA * t * bulletSpeed + t * targetVelocity.y + delta.y;

        jacobian._00 = -cosA * bulletSpeed + targetVelocity.x;
        jacobian._01 = sinA * t * bulletSpeed;
        jacobian._10 = -sinA * bulletSpeed + targetVelocity.y;
        jacobian._11 = -cosA * t * bulletSpeed;

        if (!jacobian.GetInverse(invJacobian))
        {
            return false;
        }

        cur = prev - invJacobian * f;
    }

    outDist = dist;
    outAngle = cur.y;

    return true;
}

} // unnamed namespace

BotTaskSystem::BotTaskSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<AttackTaskComponent>() | ComponentUtils::MakeMask<MoveToPointTaskComponent>())
    , taskStorage(this)
{
}

void BotTaskSystem::ProcessFixed(float32 timeElapsed)
{
    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    if (!netTimeComp->IsInitialized())
    {
        return;
    }

    currentDigitalActions.clear();

    taskStorage.DispatchTasks(timeElapsed);

    Scene* scene = GetScene();
    ActionsSingleComponent* actSingleComp = scene->GetSingletonComponent<ActionsSingleComponent>();
    int32 localPlayerId = actSingleComp->GetLocalPlayerId();
    if (localPlayerId)
    {
        for (const FastName& action : currentDigitalActions)
        {
            actSingleComp->AddDigitalAction(action, localPlayerId);
        }
    }
}

void BotTaskSystem::RegisterComponent(Entity* entity, Component* component)
{
    taskStorage.Add(component);
}

void BotTaskSystem::UnregisterComponent(Entity* entity, Component* component)
{
    taskStorage.Remove(component);
}

bool BotTaskSystem::GetShotParams(float& outYawCorrection, float& outDist, float32 timeElapsed, AttackTaskComponent* task)
{
    Entity* target = GetScene()->GetEntityByID(task->targetID);
    if (!target)
    {
        task->haveTargetPosition = false;
        return false;
    }

    Vector3 targetPos3 = target->GetWorldTransform().GetTranslationVector();
    Vector2 targetPos(targetPos3.x, targetPos3.y);

    if (!task->haveTargetPosition)
    {
        task->targetPosition = targetPos3;
        task->haveTargetPosition = true;
    }

    Vector2 prevTargetPos(task->targetPosition.x, task->targetPosition.y);
    task->targetPosition = targetPos3;

    Entity* entity = task->GetEntity();
    const Matrix4& m = entity->GetWorldTransform();
    Vector3 selfPos3 = entity->GetWorldTransform().GetTranslationVector();
    Vector2 selfPos(selfPos3.x, selfPos3.y);

    float dist;
    float shotYaw;
    float bulletSpeed = ShootComponent::MOVE_SPEED;
    Vector2 targetVelocity = (targetPos - prevTargetPos) / timeElapsed;

    if (!ComputeShotYawWithDeflection(shotYaw, dist, selfPos, bulletSpeed, targetPos, targetVelocity))
    {
        return false;
    }

    Vector2 idealDir(cosf(shotYaw), sinf(shotYaw));
    Vector2 forward(m._10, m._11);
    Vector2 side(m._00, m._01);
    outYawCorrection = atan2f(side.DotProduct(idealDir), forward.DotProduct(idealDir));
    outDist = dist;

    return true;
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, AttackTaskComponent* task)
{
    if (task->status != BotTaskStatus::IN_PROGRESS)
    {
        return;
    }

    Entity* target = GetScene()->GetEntityByID(task->targetID);
    if (!target)
    {
        task->status = BotTaskStatus::FAILURE;
        return;
    }

    HealthComponent* health = target->GetComponent<HealthComponent>();
    if (!health)
    {
        task->status = BotTaskStatus::FAILURE;
        return;
    }

    if (health->GetHealth() == 0)
    {
        task->status = BotTaskStatus::SUCCESS;
        return;
    }

    float yawCorrection;
    float dist;
    if (!GetShotParams(yawCorrection, dist, timeElapsed, task))
    {
        task->status = BotTaskStatus::FAILURE;
        return;
    }

    float absYawCorrection = fabsf(yawCorrection);

    if (dist < ShootComponent::MAX_DISTANCE - 10.f)
    {
        if (dist > 10.f)
        {
            if ((absYawCorrection < 0.5) && (absYawCorrection * dist < 2.f) && (task->delay <= 0.f))
            {
                currentDigitalActions.push_back(FastName("FIRST_SHOOT"));
                task->delay = task->reloadTime;
            }
        }
        else
        {
            if (absYawCorrection < 0.2f)
            {
                currentDigitalActions.push_back(FastName("DOWN"));
            }
        }
    }
    else
    {
        if (absYawCorrection < 0.2f)
        {
            currentDigitalActions.push_back(FastName("UP"));
        }
    }

    if (absYawCorrection > 0.005f)
    {
        if (yawCorrection < 0.f)
        {
            currentDigitalActions.push_back(FastName("LEFT"));
        }
        else
        {
            currentDigitalActions.push_back(FastName("RIGHT"));
        }
    }

    task->delay -= timeElapsed;
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, MoveToPointTaskComponent* task)
{
    if (task->status != BotTaskStatus::IN_PROGRESS)
    {
        return;
    }

    Entity* entity = task->GetEntity();
    const Matrix4& m = entity->GetWorldTransform();

    Vector3 delta3 = task->targetPoint - m.GetTranslationVector();
    Vector2 delta(delta3.x, delta3.y);
    if (delta.SquareLength() < (task->precision * task->precision))
    {
        task->status = BotTaskStatus::SUCCESS;
        return;
    }

    delta.Normalize();
    Vector2 forward = MultiplyVectorMat2x2(Vector2(0.f, 1.f), m);
    Vector2 side = MultiplyVectorMat2x2(Vector2(1.f, 0.f), m);

    float angle = atan2f(side.DotProduct(delta), forward.DotProduct(delta));
    float absAngle = fabsf(angle);

    if (absAngle < 0.1f)
    {
        currentDigitalActions.push_back(FastName("UP"));
    }

    if (absAngle > 0.01f)
    {
        if (angle < 0.f)
        {
            currentDigitalActions.push_back(FastName("LEFT"));
        }
        else
        {
            currentDigitalActions.push_back(FastName("RIGHT"));
        }
    }
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, WaitTaskComponent* task)
{
    if (task->status != BotTaskStatus::IN_PROGRESS)
    {
        return;
    }

    switch (task->type)
    {
    case WaitTaskComponent::Type::DELAY:
    {
        task->time -= timeElapsed;
        if (task->time <= 0.f)
        {
            task->status = BotTaskStatus::SUCCESS;
        }
        break;
    }
    case WaitTaskComponent::Type::TIMESTAMP:
    {
        const NetworkTimeSingleComponent* time = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
        float32 serverTime = time->GetLastServerFrameId() / time->GetFps();
        if (serverTime >= task->time)
        {
            task->status = BotTaskStatus::SUCCESS;
        }
        break;
    }
    }
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, CompositeTaskComponent* task)
{
    if (task->status != BotTaskStatus::IN_PROGRESS)
    {
        return;
    }

    switch (task->type)
    {
    case CompositeTaskComponent::Type::AND:
    {
        BotTaskStatus firstStatus = task->first->status;
        BotTaskStatus secondStatus = task->second->status;
        if ((firstStatus == BotTaskStatus::SUCCESS) && (secondStatus == BotTaskStatus::SUCCESS))
        {
            task->status = BotTaskStatus::SUCCESS;
        }
        else if ((firstStatus == BotTaskStatus::FAILURE) || (secondStatus == BotTaskStatus::FAILURE))
        {
            task->status = BotTaskStatus::FAILURE;
        }
        break;
    }
    case CompositeTaskComponent::Type::OR:
    {
        BotTaskStatus firstStatus = task->first->status;
        BotTaskStatus secondStatus = task->second->status;
        if ((firstStatus == BotTaskStatus::SUCCESS) || (secondStatus == BotTaskStatus::SUCCESS))
        {
            task->status = BotTaskStatus::SUCCESS;
        }
        else if ((firstStatus == BotTaskStatus::FAILURE) && (secondStatus == BotTaskStatus::FAILURE))
        {
            task->status = BotTaskStatus::FAILURE;
        }
        break;
    }
    }
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, RandomMovementTaskComponent* task)
{
    const float32 CHANGE_KEY_CHANCE = 0.02f;

    currentDigitalActions.push_back(FastName("UP"));
    if (Random::Instance()->RandFloat() < CHANGE_KEY_CHANCE)
    {
        task->turnLeft = !task->turnLeft;
    }
    if (Random::Instance()->RandFloat() < CHANGE_KEY_CHANCE)
    {
        task->turnRight = !task->turnRight;
    }

    if (task->turnLeft)
    {
        currentDigitalActions.push_back(FastName("LEFT"));
    }
    if (task->turnRight)
    {
        currentDigitalActions.push_back(FastName("RIGHT"));
    }
}
