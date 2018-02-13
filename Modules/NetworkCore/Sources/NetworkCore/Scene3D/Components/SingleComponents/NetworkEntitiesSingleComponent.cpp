#include "NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Snapshot.h"
#include <Scene3D/Scene.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkEntitiesSingleComponent)
{
    ReflectionRegistrator<NetworkEntitiesSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

NetworkEntitiesSingleComponent::NetworkEntitiesSingleComponent()
    : nextUniqueEntityID(1)
{
}

void NetworkEntitiesSingleComponent::RegisterEntity(NetworkID networkID, Entity* entity)
{
    auto findIt = networkEntities.find(networkID);
    if (findIt == networkEntities.end())
    {
        networkEntities.emplace(networkID, entity);
    }
    else
    {
        Entity* netEntity = findIt->second;

        if (netEntity != entity && netEntity->GetScene() != nullptr)
        {
            // the scene already has an entity with the same network ID
            // for example: if we are loading a map with static objects on the client,
            findIt->second = entity; // but replication system has replicated static objects from server
            netEntity->GetScene()->RemoveNode(netEntity); // we have to remove the old entity from scene
        }
    }
}

void NetworkEntitiesSingleComponent::UnregisterEntity(NetworkID networkID)
{
    networkEntities.erase(networkID);
}

Entity* NetworkEntitiesSingleComponent::FindByID(NetworkID networkID) const
{
    const auto networkIt = networkEntities.find(networkID);
    if (networkIt != networkEntities.end())
    {
        return networkIt->second;
    }
    return nullptr;
}

void NetworkEntitiesSingleComponent::Clear()
{
}

NetworkID NetworkEntitiesSingleComponent::GetNextUniqueEntityID()
{
#ifdef USE_SNAPSHOT_SYSTEM
    DVASSERT(false);
#endif

    ++nextUniqueEntityID;
    return nextUniqueEntityID;
}
}
