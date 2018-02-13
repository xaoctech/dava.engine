#include "NetworkTimelineSingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkTimelineSingleComponent)
{
    ReflectionRegistrator<NetworkTimelineSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

void NetworkTimelineSingleComponent::SetClientJustPaused(bool value)
{
    clientJustPaused = value;
}

bool NetworkTimelineSingleComponent::IsClientJustPaused() const
{
    return clientJustPaused;
}

void NetworkTimelineSingleComponent::SetServerJustPaused(bool value)
{
    serverJustPaused = value;
}

bool NetworkTimelineSingleComponent::IsServerJustPaused() const
{
    return serverJustPaused;
}

void NetworkTimelineSingleComponent::SetStepOver(bool value)
{
    stepOver = value;
}

bool NetworkTimelineSingleComponent::HasStepOver() const
{
    return stepOver;
}

void NetworkTimelineSingleComponent::SetStepsCount(int32 value)
{
    stepsCount = value;
}

int32 NetworkTimelineSingleComponent::GetStepsCount() const
{
    return stepsCount;
}

void NetworkTimelineSingleComponent::Clear()
{
}
}
