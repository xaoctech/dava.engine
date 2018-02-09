#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SingletonComponent.h"
#include "Reflection/Reflection.h"
#include "NetworkCore/NetworkTypes.h"

namespace DAVA
{
class NetworkVisibilitySingleComponent : public SingletonComponent
{
public:
    using EntityToFrequency = UnorderedMap<const Entity*, uint8>;

    DAVA_VIRTUAL_REFLECTION(NetworkVisibilitySingleComponent, SingletonComponent);

    void SetVisibility(NetworkPlayerID observerPlayerID, const Entity* target, uint8 frequency);
    uint8 GetVisibility(NetworkPlayerID observerPlayerID, const Entity* target) const;

    const EntityToFrequency& GetVisibleEntities(NetworkPlayerID observerPlayerID) const;
    const Vector<NetworkID>& GetVisibleNetworkIDs(NetworkPlayerID observerPlayerID) const;

    const UnorderedSet<const Entity*>& GetAddedEntities(NetworkPlayerID observerPlayerID) const;
    const UnorderedSet<const Entity*>& GetRemovedEntities(NetworkPlayerID observerPlayerID) const;

    void ClearCache();

    UnorderedMap<NetworkPlayerID, UnorderedSet<const Entity*>> playerToAddedEntities;
    UnorderedMap<NetworkPlayerID, UnorderedSet<const Entity*>> playerToRemovedEntities;

private:
    UnorderedMap<NetworkPlayerID, EntityToFrequency> observerPlayerToFrequency;
    UnorderedMap<NetworkPlayerID, Vector<NetworkID>> observerPlayerToNetworkIDs;
    const EntityToFrequency emptyEntityToFrequency;
    const UnorderedSet<const Entity*> emptyEntities;
    const Vector<NetworkID> emptyNetworkIDs;
};

} //namespace DAVA
