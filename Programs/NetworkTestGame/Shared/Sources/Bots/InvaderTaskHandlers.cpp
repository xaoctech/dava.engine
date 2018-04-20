#include "BotTaskSystem.h"

using namespace DAVA;

void BotTaskSystem::ProcessTask(float32 timeElapsed, InvaderSlideToBorderTaskComponent* task)
{
    Entity* entity = task->GetEntity();
    Vector3 selfPos = entity->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();

    if (task->movingRight)
    {
        if (selfPos.x >= 30.f)
        {
            task->movingRight = false;
        }
    }
    else
    {
        if (selfPos.x <= -30.f)
        {
            task->movingRight = true;
        }
    }

    if (task->movingRight)
    {
        currentDigitalActions.push_back(FastName("RIGHT"));
    }
    else
    {
        currentDigitalActions.push_back(FastName("LEFT"));
    }
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, InvaderWagToBorderTaskComponent* task)
{
    Entity* entity = task->GetEntity();
    Vector3 selfPos = entity->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();

    if (task->movingRight)
    {
        if (selfPos.x >= 30.f)
        {
            task->movingRight = false;
        }
    }
    else
    {
        if (selfPos.x <= -30.f)
        {
            task->movingRight = true;
        }
    }

    if (task->waggingRight)
    {
        if (task->framesWithSameWagging >= (task->movingRight ? 9 : 3))
        {
            task->waggingRight = false;
            task->framesWithSameWagging = 0;
        }
    }
    else
    {
        if (task->framesWithSameWagging >= (task->movingRight ? 3 : 9))
        {
            task->waggingRight = true;
            task->framesWithSameWagging = 0;
        }
    }

    if (task->waggingRight)
    {
        currentDigitalActions.push_back(FastName("RIGHT"));
    }
    else
    {
        currentDigitalActions.push_back(FastName("LEFT"));
    }
    currentDigitalActions.push_back(FastName("ACCELERATE"));

    task->framesWithSameWagging++;
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, InvaderDodgeCenterTaskComponent* task)
{
    Entity* entity = task->GetEntity();
    Vector3 selfPos = entity->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();

    if (task->movingRight)
    {
        if (selfPos.x >= -2.f)
        {
            task->movingRight = false;
        }
    }
    else
    {
        if (selfPos.x <= -30.f)
        {
            task->movingRight = true;
        }
    }

    if (task->movingRight)
    {
        currentDigitalActions.push_back(FastName("RIGHT"));
    }
    else
    {
        currentDigitalActions.push_back(FastName("LEFT"));
    }
}

void BotTaskSystem::ProcessTask(float32 timeElapsed, InvaderShootIfSeeingTargetTaskComponent* task)
{
    if (task->noAmmo)
    {
        return;
    }

    Entity* target = GetScene()->GetEntityByID(task->targetId);
    if (!target)
    {
        return;
    }

    Vector3 shooterPos = task->GetEntity()->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();
    Vector3 targetPos = target->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslation();

    if (fabs(shooterPos.x - targetPos.x) < 0.5f)
    {
        currentDigitalActions.push_back(FastName("FIRST_SHOOT"));
        task->noAmmo = true;
    }
}
