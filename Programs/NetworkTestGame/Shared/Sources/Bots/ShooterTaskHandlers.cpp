#include "BotTaskSystem.h"

#include "ShooterConstants.h"
#include "ShooterUtils.h"

#include "Components/HealthComponent.h"
#include "Components/ShooterCarUserComponent.h"

#include <NetworkCore/NetworkTypes.h>

//#include <Physics/Controllers/CapsuleCharacterControllerComponent.h>

using namespace DAVA;

namespace
{
void ComputeYawPitchFromVectors(const Vector3& src, const Vector3& dst, float32& yaw, float32& pitch)
{
    Vector2 dstFlat(dst.x, dst.y);
    dstFlat.Normalize();
    Vector2 srcFlat(src.x, src.y);
    srcFlat.Normalize();
    Vector2 srcFlatOrt(-srcFlat.y, srcFlat.x);
    yaw = atan2f(dstFlat.DotProduct(srcFlatOrt), dstFlat.DotProduct(srcFlat));

    Quaternion yawRotation = Quaternion::MakeRotationFastZ(yaw);
    Vector3 srcRotated = yawRotation.ApplyToVectorFast(src);
    Vector3 ort = srcRotated.CrossProduct(Vector3(0.f, 0.f, 1.f));
    ort.Normalize();
    Vector3 srcRotatedOrt = srcRotated.CrossProduct(ort);
    pitch = atan2f(dst.DotProduct(srcRotatedOrt), dst.DotProduct(srcRotated));
}
} // unnamed namespace

void BotTaskSystem::RotateTowardsTarget(const ShooterAimComponent& aimComponent, const Vector3& targetPosition)
{
    Vector3 aimRayOrigin;
    Vector3 aimRayDirection;
    Vector3 aimRayEnd;
    Entity* aimEndEntity;
    GetCurrentAimRay(aimComponent, RaycastFilter::IGNORE_SOURCE | RaycastFilter::IGNORE_DYNAMICS, aimRayOrigin, aimRayDirection, aimRayEnd, &aimEndEntity);

    Vector3 targetDirection = targetPosition - aimRayOrigin;
    targetDirection.Normalize();

    float32 yaw, pitch;
    ComputeYawPitchFromVectors(aimRayDirection, targetDirection, yaw, pitch);

    if (fabsf(yaw) > 0.5f * DEG_TO_RAD || fabsf(pitch) > 0.5f * DEG_TO_RAD)
    {
        float32 deltaX = Clamp(-yaw * RAD_TO_DEG * 0.2f, -1.f, 1.f);
        float32 deltaY = Clamp(pitch * RAD_TO_DEG * 0.05f, -1.f, 1.f);

        currentAnalogActions.emplace_back(SHOOTER_ACTION_ANALOG_ROTATE, Vector2(deltaX, deltaY));
    }
}

BotTaskStatus BotTaskSystem::UpdateAttackTaskStatus(Entity* target)
{
    if (!target)
    {
        return BotTaskStatus::FAILURE;
    }

    HealthComponent* health = target->GetComponent<HealthComponent>();
    if (!health)
    {
        return BotTaskStatus::FAILURE;
    }

    if (health->GetHealth() == 0)
    {
        return BotTaskStatus::SUCCESS;
    }

    return BotTaskStatus::IN_PROGRESS;
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, ShooterAttackStandingStillTaskComponent* task)
{
    Entity* target = GetScene()->GetEntityByID(task->targetId);

    task->status = UpdateAttackTaskStatus(target);
    if (task->status != BotTaskStatus::IN_PROGRESS)
    {
        return;
    }

    ShooterAimComponent* aimComponent = task->GetEntity()->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent);
    Vector3 targetPos = target->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation() + SHOOTER_CHARACTER_AIM_TO;
    RotateTowardsTarget(*aimComponent, targetPos);

    static uint32 shootCounter = 0;
    if (shootCounter++ % SHOOTER_SHOOT_COOLDOWN_FRAMES == 0)
    {
        currentDigitalActions.push_back(SHOOTER_ACTION_ATTACK_BULLET);
    }
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, ShooterAttackPursuingTargetTaskComponent* task)
{
    Entity* target = GetScene()->GetEntityByID(task->targetId);

    task->status = UpdateAttackTaskStatus(target);
    if (task->status != BotTaskStatus::IN_PROGRESS)
    {
        return;
    }

    ShooterAimComponent* aimComponent = task->GetEntity()->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent);
    Vector3 targetPos = target->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation() + SHOOTER_CHARACTER_AIM_TO;
    RotateTowardsTarget(*aimComponent, targetPos);

    Vector3 actorPos = task->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();
    if (task->forward && (targetPos - actorPos).SquareLength() < 100)
    {
        task->forward = false;
    }

    if (!task->forward && (targetPos - actorPos).SquareLength() > 900)
    {
        task->forward = true;
    }

    currentDigitalActions.push_back(task->forward ? SHOOTER_ACTION_MOVE_FORWARD : SHOOTER_ACTION_MOVE_BACKWARD);

    static uint32 shootCounter = 0;
    if (shootCounter++ % SHOOTER_SHOOT_COOLDOWN_FRAMES == 0)
    {
        currentDigitalActions.push_back(SHOOTER_ACTION_ATTACK_BULLET);
    }
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, ShooterAttackCirclingAroundTaskComponent* task)
{
    Entity* target = GetScene()->GetEntityByID(task->targetId);

    task->status = UpdateAttackTaskStatus(target);
    if (task->status != BotTaskStatus::IN_PROGRESS)
    {
        return;
    }

    ShooterAimComponent* aimComponent = task->GetEntity()->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent);
    Vector3 targetPos = target->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation() + SHOOTER_CHARACTER_AIM_TO;
    RotateTowardsTarget(*aimComponent, targetPos);

    currentDigitalActions.push_back(task->right ? SHOOTER_ACTION_MOVE_RIGHT : SHOOTER_ACTION_MOVE_LEFT);

    static uint32 shootCounter = 0;
    if (shootCounter++ % SHOOTER_SHOOT_COOLDOWN_FRAMES == 0)
    {
        currentDigitalActions.push_back(SHOOTER_ACTION_ATTACK_BULLET);
    }
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, ShooterAttackWaggingTaskComponent* task)
{
    Entity* target = GetScene()->GetEntityByID(task->targetId);

    task->status = UpdateAttackTaskStatus(target);
    if (task->status != BotTaskStatus::IN_PROGRESS)
    {
        return;
    }

    ShooterAimComponent* aimComponent = task->GetEntity()->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent);
    Vector3 targetPos = target->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation() + SHOOTER_CHARACTER_AIM_TO;
    RotateTowardsTarget(*aimComponent, targetPos);

    if (task->sameDirectionTime > 1.f)
    {
        task->right = !task->right;
        task->sameDirectionTime = 0;
    }
    task->sameDirectionTime += timeElapsed;

    currentDigitalActions.push_back(task->right ? SHOOTER_ACTION_MOVE_RIGHT : SHOOTER_ACTION_MOVE_LEFT);

    static uint32 shootCounter = 0;
    if (shootCounter++ % SHOOTER_SHOOT_COOLDOWN_FRAMES == 0)
    {
        currentDigitalActions.push_back(SHOOTER_ACTION_ATTACK_BULLET);
    }
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, ShooterMoveToPointShortestTaskComponent* task)
{
    if (task->status != BotTaskStatus::IN_PROGRESS)
    {
        return;
    }

    Vector3 actorPos = task->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();
    Vector3 delta = task->point - actorPos;
    if (Vector2(delta.x, delta.y).SquareLength() < 9.f)
    {
        task->status = BotTaskStatus::SUCCESS;
    }

    ShooterAimComponent* aimComponent = task->GetEntity()->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent);
    RotateTowardsTarget(*aimComponent, task->point);

    currentDigitalActions.push_back(SHOOTER_ACTION_MOVE_FORWARD);

    if (task->accelerate)
    {
        currentDigitalActions.push_back(SHOOTER_ACTION_ACCELERATE);
    }

    if (task->accelerateTime > 4.f)
    {
        task->accelerate = !task->accelerate;
        task->accelerateTime = 0.f;
    }
    task->accelerateTime += timeElapsed;
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, ShooterMoveToPointWindingTaskComponent* task)
{
    if (task->status != BotTaskStatus::IN_PROGRESS)
    {
        return;
    }

    Vector3 actorPos = task->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();
    Vector3 delta = task->point - actorPos;
    if (Vector2(delta.x, delta.y).SquareLength() < 9.f)
    {
        task->status = BotTaskStatus::SUCCESS;
    }

    ShooterAimComponent* aimComponent = task->GetEntity()->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent);
    RotateTowardsTarget(*aimComponent, task->point);

    currentDigitalActions.push_back(SHOOTER_ACTION_MOVE_FORWARD);

    if (task->sameDirectionTime > 3.f)
    {
        task->right = !task->right;
        task->sameDirectionTime = 0;
    }
    task->sameDirectionTime += timeElapsed;

    currentDigitalActions.push_back(task->right ? SHOOTER_ACTION_MOVE_RIGHT : SHOOTER_ACTION_MOVE_LEFT);
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, ShooterHangAroundTaskComponent* task)
{
    if (task->sameDirectionTime > 5.f)
    {
        const Transform& transform = task->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform();
        Vector3 actorPos = transform.GetTranslation();
        Quaternion actorRot = transform.GetRotation();
        Quaternion rotation = actorRot * Quaternion::MakeRotationFastZ(task->angle);
        Vector3 direction = rotation.ApplyToVectorFast(SHOOTER_CHARACTER_FORWARD);

        task->point = actorPos + direction * 100.f;
        task->sameDirectionTime = 0;
    }
    task->sameDirectionTime += timeElapsed;

    ShooterAimComponent* aimComponent = task->GetEntity()->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent);
    RotateTowardsTarget(*aimComponent, task->point);

    currentDigitalActions.push_back(SHOOTER_ACTION_MOVE_FORWARD);
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, ShooterDriveTaskComponent* task)
{
    if (task->status != BotTaskStatus::IN_PROGRESS)
    {
        return;
    }

    Entity* car = GetScene()->GetEntityByID(task->carId);
    if (!car)
    {
        task->status = BotTaskStatus::FAILURE;
        return;
    }

    ShooterCarUserComponent* carUserComponent = task->GetEntity()->GetComponent<ShooterCarUserComponent>();
    DVASSERT(carUserComponent);
    bool isInside = carUserComponent->GetCarNetworkId() != NetworkID::INVALID;

    if (!isInside)
    {
        Vector3 actorPos = task->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();
        Vector3 carPos = car->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();
        if ((carPos - actorPos).Length() > SHOOTER_CAR_MAX_INTERACT_DISTANCE)
        {
            task->status = BotTaskStatus::FAILURE;
            return;
        }

        // get in
        if (task->actionTime < 1.f)
        {
            ShooterAimComponent* aimComponent = task->GetEntity()->GetComponent<ShooterAimComponent>();
            DVASSERT(aimComponent);

            RotateTowardsTarget(*aimComponent, carPos);
        }
        else
        {
            currentDigitalActions.push_back(SHOOTER_ACTION_INTERACT);
            task->actionTime = 0.f;
        }
    }
    else
    {
        // drive to point
        const Matrix4& m = car->GetComponent<TransformComponent>()->GetWorldMatrix();
        Vector3 delta3 = task->point - m.GetTranslationVector();
        Vector2 delta(delta3.x, delta3.y);
        float32 squareDist = delta.SquareLength();
        delta.Normalize();

        Vector2 forward(m._00, m._01); // TODO: model has wrong forward axis: (-1, 0, 0)
        Vector2 side(m._10, m._11);
        float32 angle = atan2f(side.DotProduct(delta), -forward.DotProduct(delta));

        if (fabsf(angle) > 0.2f)
        {
            currentDigitalActions.push_back(angle < 0.f ? SHOOTER_ACTION_MOVE_LEFT : SHOOTER_ACTION_MOVE_RIGHT);
        }

        currentDigitalActions.push_back(SHOOTER_ACTION_MOVE_FORWARD);

        // get out
        if (squareDist < 25.f || task->actionTime > 20.f)
        {
            currentDigitalActions.push_back(SHOOTER_ACTION_INTERACT);
            task->actionTime = 0.f;

            task->status = BotTaskStatus::SUCCESS;
        }
    }

    task->actionTime += timeElapsed;
}