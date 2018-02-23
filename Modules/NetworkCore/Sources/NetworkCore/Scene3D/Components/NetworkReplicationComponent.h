#pragma once

#include "NetworkCore/NetworkTypes.h"
#include "Reflection/Reflection.h"
#include "Debug/DVAssert.h"
#include "Entity/Component.h"
#include "Base/Introspection.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Math/Vector.h"

#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"

namespace DAVA
{
enum class EntityType : uint8
{
    NONE,
    VEHICLE,
    BULLET,
    // TODO: move these into separate component for GameModeSystemPhysics
    BIG_CUBE,
    SMALL_CUBE,
    ANY
};

class NetworkReplicationComponent : public Component
{
public:
    NetworkReplicationComponent();
    ~NetworkReplicationComponent()
    {};

    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetNetworkID(NetworkID networkUniqueID_);
    NetworkID GetNetworkID() const;

    const uint8 GetOwnerTeamID() const;
    void SetOwnerTeamID(uint8 teamID);

    NetworkPlayerID GetNetworkPlayerID() const;
    void SetNetworkPlayerID(NetworkPlayerID networkPlayerID_);

    // SERVER_COMPLETE
    void SetEntityType(EntityType entityType_);
    EntityType GetEntityType() const;

    void SetFrameID(uint32 frameID);
    uint32 GetFrameID() const;

    void SetEntityIndexInFrame(uint8 entityIndexInFrame_);
    uint8 GetEntityIndexInFrame() const;

    void SetEntityCountInFrame(uint8 entityCountInFrame_);
    uint8 GetEntityCountInFrame() const;

private:
    NetworkPlayerID networkPlayerID = 0;
    EntityType entityType;
    NetworkID networkUniqueID = NetworkID::INVALID;
    uint8 teamID = 0;
    uint32 frameID = 0;
    uint8 entityIndexInFrame = 0;
    uint8 entityCountInFrame = 0;

public:
    DAVA_VIRTUAL_REFLECTION(NetworkReplicationComponent, Component);
};

REGISTER_CLASS(NetworkReplicationComponent);

// Implementation
inline void NetworkReplicationComponent::SetNetworkID(NetworkID networkUniqueID_)
{
    DVASSERT(networkUniqueID == NetworkID::INVALID || networkUniqueID_ == networkUniqueID);
    networkUniqueID = networkUniqueID_;
}

inline NetworkID NetworkReplicationComponent::GetNetworkID() const
{
    return networkUniqueID;
}

inline NetworkPlayerID NetworkReplicationComponent::GetNetworkPlayerID() const
{
    return networkPlayerID;
}

inline void NetworkReplicationComponent::SetNetworkPlayerID(NetworkPlayerID networkPlayerID_)
{
    networkPlayerID = networkPlayerID_;
}

inline const uint8 NetworkReplicationComponent::GetOwnerTeamID() const
{
    return teamID;
}

inline void NetworkReplicationComponent::SetOwnerTeamID(uint8 teamID_)
{
    teamID = teamID_;
}

inline void NetworkReplicationComponent::SetFrameID(uint32 frameID_)
{
    frameID = frameID_;
}
inline uint32 NetworkReplicationComponent::GetFrameID() const
{
    return frameID;
}

inline void NetworkReplicationComponent::SetEntityIndexInFrame(uint8 entityIndexInFrame_)
{
    entityIndexInFrame = entityIndexInFrame_;
}

inline uint8 NetworkReplicationComponent::GetEntityIndexInFrame() const
{
    return entityIndexInFrame;
}

inline void NetworkReplicationComponent::SetEntityCountInFrame(uint8 entityCountInFrame_)
{
    entityCountInFrame = entityCountInFrame_;
}
inline uint8 NetworkReplicationComponent::GetEntityCountInFrame() const
{
    return entityCountInFrame;
}
}
