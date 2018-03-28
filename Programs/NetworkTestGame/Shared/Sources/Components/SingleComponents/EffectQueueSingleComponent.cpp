#include "EffectQueueSingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(EffectQueueSingleComponent)
{
    ReflectionRegistrator<EffectQueueSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

EffectQueueSingleComponent::EffectQueueSingleComponent()
    : ClearableSingleComponent(ClearableSingleComponent::Usage::FixedProcesses)
{
}

void EffectQueueSingleComponent::Clear()
{
    effects.clear();
}

EffectDescriptor& EffectQueueSingleComponent::CreateEffect(int resourceId)
{
    effects.emplace_back(resourceId);
    return effects.back();
}
