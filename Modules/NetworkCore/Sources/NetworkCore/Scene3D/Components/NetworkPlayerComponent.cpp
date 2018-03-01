#include "NetworkPlayerComponent.h"
#include "NetworkReplicationComponent.h"

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

void NetworkPlayerComponent::SetSendPeriod(const Entity* target, uint8 period)
{
    auto idxIt = entityToInternalIndex.find(target);

    if (idxIt == entityToInternalIndex.end()) // target not in cache
    {
        if (period > 0) // becomes visible
        {
            entityToInternalIndex[target] = periods.size();

            periods.emplace_back();
            periods.back().period = period;
            periods.back().target = target;

            NetworkID networkID = target->GetComponent<NetworkReplicationComponent>()->GetNetworkID();
            visibleEntityIds.push_back(networkID);
        }
    }
    else // target present in cache
    {
        size_t idx = idxIt->second;

        if (period > 0) // still visible, update period
        {
            periods[idx].period = period;
        }
        else // becomes invisible
        {
            visibleEntityIds[idx] = visibleEntityIds.back();
            visibleEntityIds.pop_back();

            SendPeriodInfo& replacedEntry = periods[idx];
            SendPeriodInfo& movedEntry = periods.back();

            entityToInternalIndex[movedEntry.target] = idx;
            entityToInternalIndex.erase(replacedEntry.target);

            replacedEntry = movedEntry;
            periods.pop_back();
        }
    }
}
}
