#include "NetworkDeltaSingleComponent.h"
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkDeltaSingleComponent)
{
    ReflectionRegistrator<NetworkDeltaSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

NetworkDeltaSingleComponent::NetworkDeltaSingleComponent()
    : ClearableSingleComponent(ClearableSingleComponent::Usage::FixedProcesses)
{
}

void NetworkDeltaSingleComponent::Clear()
{
    deltas.clear();
}
}
