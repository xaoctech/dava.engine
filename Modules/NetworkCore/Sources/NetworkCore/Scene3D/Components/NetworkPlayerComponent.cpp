#include "NetworkPlayerComponent.h"

#include <Engine/Engine.h>
#include <Entity/ComponentManager.h>
#include <Reflection/ReflectionRegistrator.h>

#include <limits>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkPlayerComponent)
{
    ReflectionRegistrator<NetworkPlayerComponent>::Begin()[M::Replicable(M::Privacy::PRIVATE)]
    .ConstructorByPointer()
    .Field("visibleEntityIds", &NetworkPlayerComponent::visibleEntityIds)[M::Replicable()]
    .End();
}

NetworkPlayerComponent::NetworkPlayerComponent()
    : visibleEntityIds(MAX_NETWORK_VISIBLE_ENTITIES_COUNT)
{
}

Component* NetworkPlayerComponent::Clone(Entity* toEntity)
{
    return new NetworkPlayerComponent(*this);
}
}
