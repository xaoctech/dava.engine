#include "NetworkReplicationComponent.h"
#include "NetworkCore/Snapshot.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkReplicationComponent)
{
    ReflectionRegistrator<NetworkReplicationComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .Field("replicableMask", &NetworkReplicationComponent::replicableMask)[M::Replicable()]
    .End();
}

NetworkReplicationComponent::NetworkReplicationComponent(NetworkID nid)
    : id(nid)
{
    DVASSERT(id.IsValid());
}

Component* NetworkReplicationComponent::Clone(Entity* toEntity)
{
    NetworkReplicationComponent* rc = new NetworkReplicationComponent(id);
    rc->SetEntity(toEntity);
    rc->id = id;
    rc->replicableMask = replicableMask;
    rc->replicationPrivacyMask = replicationPrivacyMask;

    return rc;
}

void NetworkReplicationComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    DVASSERT(false);
    // TODO:
    // ...
}

void NetworkReplicationComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    DVASSERT(false);
    // TODO:
    // ...
}

NetworkID NetworkReplicationComponent::GetNetworkID() const
{
    return id;
}

NetworkPlayerID NetworkReplicationComponent::GetNetworkPlayerID() const
{
    DVASSERT(id.IsPlayerId() || id.IsStaticId());
    if (id.IsPlayerId())
    {
        return id.GetPlayerId();
    }
    return 0;
}

const ComponentMask& NetworkReplicationComponent::GetReplicationMask() const
{
    return replicableMask;
}

const ComponentMask& NetworkReplicationComponent::GetReplicationPrivacyMask() const
{
    return replicationPrivacyMask;
}

void NetworkReplicationComponent::SetForReplication(const Type* componentType, M::Privacy privacy)
{
    DVASSERT(entity == nullptr, "You can't setup replicating when component is already belong to entity");

    replicableMask.Set(componentType);
    if (privacy > M::Privacy::PRIVATE)
    {
        replicationPrivacyMask.Set(componentType);
    }
}
}
