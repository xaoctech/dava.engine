#include "NetworkReplicationComponent.h"
#include "NetworkCore/Snapshot.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkReplicationComponent)
{
    ReflectionRegistrator<NetworkReplicationComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .Field("networkPlayerID", &NetworkReplicationComponent::GetNetworkPlayerID, &NetworkReplicationComponent::SetNetworkPlayerID)[M::Replicable()]
    .Field("entityType", &NetworkReplicationComponent::GetEntityType, &NetworkReplicationComponent::SetEntityType)[M::Replicable()]
    .Field("frameID", &NetworkReplicationComponent::GetFrameID, &NetworkReplicationComponent::SetFrameID)[M::Replicable()]
    .Field("networkID", &NetworkReplicationComponent::GetNetworkID, &NetworkReplicationComponent::SetNetworkID)[M::Replicable()]
    .Field("entityIndexInFrame", &NetworkReplicationComponent::GetEntityIndexInFrame, &NetworkReplicationComponent::SetEntityIndexInFrame)[M::Replicable()]
    .Field("entityCountInFrame", &NetworkReplicationComponent::GetEntityCountInFrame, &NetworkReplicationComponent::SetEntityCountInFrame)[M::Replicable()]
    .End();
}

NetworkReplicationComponent::NetworkReplicationComponent()
{
}

Component* NetworkReplicationComponent::Clone(Entity* toEntity)
{
    NetworkReplicationComponent* rc = new NetworkReplicationComponent();
    rc->SetEntity(toEntity);
    rc->SetNetworkID(GetNetworkID());
    rc->SetNetworkPlayerID(GetNetworkPlayerID());
    rc->SetOwnerTeamID(GetOwnerTeamID());
    return rc;
}

void NetworkReplicationComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    if (archive != nullptr)
    {
        DVASSERT(GetNetworkPlayerID() != 0);
        archive->SetUInt32("nrc.networkPlayerID", GetNetworkPlayerID());
    }
}

void NetworkReplicationComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    if (archive != nullptr)
    {
        NetworkPlayerID playerID = archive->GetUInt32("nrc.networkPlayerID");
        SetNetworkPlayerID(playerID);
    }
}

void NetworkReplicationComponent::SetEntityType(EntityType entityType_)
{
    entityType = entityType_;
}

EntityType NetworkReplicationComponent::GetEntityType() const
{
    return entityType;
}
}
