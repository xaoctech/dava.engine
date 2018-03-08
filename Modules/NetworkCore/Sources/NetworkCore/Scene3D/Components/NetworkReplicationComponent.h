#pragma once

#include "NetworkCore/NetworkTypes.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectedMeta.h"
#include "Debug/DVAssert.h"
#include "Entity/Component.h"
#include "Base/Introspection.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Math/Vector.h"

namespace DAVA
{
class NetworkIdSystem;
class NetworkReplicationComponent : public Component
{
    friend class NetworkIdSystem;

    DAVA_VIRTUAL_REFLECTION(NetworkReplicationComponent, Component);

public:
    NetworkReplicationComponent(NetworkID id);

    NetworkID GetNetworkID() const;
    NetworkPlayerID GetNetworkPlayerID() const;

    const ComponentMask& GetReplicationMask() const;
    const ComponentMask& GetReplicationPrivacyMask() const;

    template <typename T>
    void SetForReplication(M::Privacy privacy);
    void SetForReplication(const Type* componentType, M::Privacy privacy);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

protected:
    NetworkID id = NetworkID::INVALID;
    ComponentMask replicableMask;
    ComponentMask replicationPrivacyMask;
};

template <typename T>
void NetworkReplicationComponent::SetForReplication(M::Privacy privacy)
{
    SetForReplication(Type::Instance<T>(), privacy);
}
} // namespace DAVA
