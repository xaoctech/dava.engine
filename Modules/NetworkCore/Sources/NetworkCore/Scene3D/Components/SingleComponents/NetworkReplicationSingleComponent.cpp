#include "NetworkReplicationSingleComponent.h"
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkReplicationSingleComponent)
{
    ReflectionRegistrator<NetworkReplicationSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

void NetworkReplicationSingleComponent::Clear()
{
}
}
