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

void NetworkDeltaSingleComponent::Clear()
{
    deltas.clear();
}
}
