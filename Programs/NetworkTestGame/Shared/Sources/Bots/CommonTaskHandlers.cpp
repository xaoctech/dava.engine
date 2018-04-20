#include "BotTaskSystem.h"

#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>

using namespace DAVA;

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
        const NetworkTimeSingleComponent* networkTimeComponent = GetScene()->GetSingleComponentForRead<NetworkTimeSingleComponent>(this);
        float32 serverTime = networkTimeComponent->GetUptimeMs() * 1000;
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