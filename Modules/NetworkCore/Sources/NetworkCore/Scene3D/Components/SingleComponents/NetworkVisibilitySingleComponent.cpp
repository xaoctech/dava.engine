#include "NetworkVisibilitySingleComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"

namespace DAVA
{
template <typename Ret, typename Container, typename Default>
const Ret& GetByObserverPlayerID(NetworkPlayerID observerPlayerID, const Container& container, const Default& def)
{
    const auto& findIt = container.find(observerPlayerID);
    return (findIt == container.end()) ? def : findIt->second;
}

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkVisibilitySingleComponent)
{
    ReflectionRegistrator<NetworkVisibilitySingleComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .ConstructorByPointer()
    .End();
}

uint8 NetworkVisibilitySingleComponent::GetVisibility(NetworkPlayerID observerPlayerID, const Entity* target) const
{
    auto targetsFrequencyIt = observerPlayerToFrequency.find(observerPlayerID);
    if (targetsFrequencyIt == observerPlayerToFrequency.end())
    {
        return 0;
    }
    const UnorderedMap<const Entity*, uint8>& targetsFrequency = targetsFrequencyIt->second;
    const auto& targetIt = targetsFrequency.find(target);
    if (targetIt == targetsFrequency.end())
    {
        return 0;
    }

    const uint8 frequency = targetIt->second;
    return frequency;
}

void NetworkVisibilitySingleComponent::SetVisibility(NetworkPlayerID observerPlayerID, const Entity* target, uint8 frequency)
{
    EntityToFrequency& entitiesToFreqs = observerPlayerToFrequency[observerPlayerID];
    Vector<NetworkID>& entitiesIDs = observerPlayerToNetworkIDs[observerPlayerID];
    NetworkID targetID = target->GetComponent<NetworkReplicationComponent>()->GetNetworkID();
    auto findIt = entitiesToFreqs.find(target);
    if (findIt != entitiesToFreqs.end() && frequency == 0)
    {
        entitiesToFreqs.erase(findIt);
        playerToRemovedEntities[observerPlayerID].insert(target);
        entitiesIDs.erase(std::remove(entitiesIDs.begin(), entitiesIDs.end(), targetID), entitiesIDs.end());
    }
    else if (findIt == entitiesToFreqs.end() && frequency > 0)
    {
        entitiesToFreqs.emplace(target, frequency);
        playerToAddedEntities[observerPlayerID].insert(target);
        entitiesIDs.push_back(targetID);
    }
    else if (findIt != entitiesToFreqs.end() && frequency > 0)
    {
        findIt->second = frequency;
    }
}

const NetworkVisibilitySingleComponent::EntityToFrequency&
NetworkVisibilitySingleComponent::GetVisibleEntities(NetworkPlayerID observerPlayerID) const
{
    return GetByObserverPlayerID<EntityToFrequency>(observerPlayerID, observerPlayerToFrequency, emptyEntityToFrequency);
}

const Vector<NetworkID>& NetworkVisibilitySingleComponent::GetVisibleNetworkIDs(NetworkPlayerID observerPlayerID) const
{
    return GetByObserverPlayerID<Vector<NetworkID>>(observerPlayerID, observerPlayerToNetworkIDs, emptyNetworkIDs);
}

const UnorderedSet<const Entity*>&
NetworkVisibilitySingleComponent::GetAddedEntities(NetworkPlayerID observerPlayerID) const
{
    return GetByObserverPlayerID<UnorderedSet<const Entity*>>(observerPlayerID, playerToAddedEntities, emptyEntities);
}

const UnorderedSet<const Entity*>&
NetworkVisibilitySingleComponent::GetRemovedEntities(NetworkPlayerID observerPlayerID) const
{
    return GetByObserverPlayerID<UnorderedSet<const Entity*>>(observerPlayerID, playerToRemovedEntities, emptyEntities);
}

void NetworkVisibilitySingleComponent::ClearCache()
{
    for (auto& addedEntities : playerToAddedEntities)
    {
        addedEntities.second.clear();
    }
    for (auto& removedEntities : playerToRemovedEntities)
    {
        removedEntities.second.clear();
    }
}

} //namespace DAVA
