#pragma once

#include "Base/BaseTypes.h"
#include "Base/UnordererSet.h"
#include "Reflection/Reflection.h"
#include "Debug/DVAssert.h"
#include "Entity/Component.h"
#include "Base/Introspection.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/Systems/BaseSimulationSystem.h"
#include "Math/Vector.h"

#include "NetworkCore/Snapshot.h"

namespace DAVA
{
class FrameActionID
{
public:
    FrameActionID()
        : pureId(0)
    {
    }

    FrameActionID(uint32 frameId_, uint32 playerId_, uint32 actionId_)
        : frameId(frameId_)
        , playerId(playerId_)
        , actionId(actionId_)
        , flag(0)
    {
    }

    union
    {
        uint32 pureId = 0;
        struct
        {
            uint32 frameId : 20;
            uint32 playerId : 7;
            uint32 actionId : 4;
            uint32 flag : 1;
        };
    };

    /*
    FrameActionID(uint32 serverFrameID_, uint32 actionInFrameIndex_)
        : serverFrameID(serverFrameID_)
        , actionInFrameIndex(actionInFrameIndex_)
    {
#ifndef USE_SNAPSHOT_SYSTEM
#ifdef SERVER
        createdOnClient = 0;
#else
        createdOnClient = 1;
#endif
#else
        createdOnClient = 0;
#endif
    }
    // SERVER_COMPLETE

    bool operator==(const FrameActionID& other) const
    {
        return (serverFrameID == other.serverFrameID && actionInFrameIndex == other.actionInFrameIndex);
    }

    union
    {
        uint32 pureID = 0;
        struct
        {
            uint32 serverFrameID : 24; //  2^24 server frames after which server should restart frame counter
            uint32 actionInFrameIndex : 7; //  maximum 256 identical actions per frame.
            uint32 createdOnClient : 1;
        };
    };
    */
};

class NetworkPredictComponent : public Component
{
    DAVA_VIRTUAL_REFLECTION(NetworkPredictComponent, Component);

public:
    NetworkPredictComponent() = default;

    void SetFrameActionID(const FrameActionID& frameActionID);
    const FrameActionID& GetFrameActionID() const;

    void SetFrameActionIDSnap(uint32 frameActionID);
    uint32 GetFrameActionIDSnap() const;

    bool IsPredictedComponent(Component* component) const;
    bool IsPredictedComponent(const Type* componentType) const;
    void AddPredictedComponent(const Type* componentType);
    void RemovePredictedComponent(const Type* componentType);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    ComponentMask predictedComponentMask;
    FrameActionID frameActionID;
    uint32 processedFrameId = ~0u;
};

// Implementation
inline void NetworkPredictComponent::SetFrameActionID(const FrameActionID& frameActionID_)
{
    frameActionID = frameActionID_;
}

inline const FrameActionID& NetworkPredictComponent::GetFrameActionID() const
{
    return frameActionID;
}

inline void NetworkPredictComponent::SetFrameActionIDSnap(uint32 frameActionID_)
{
    frameActionID.pureId = frameActionID_;
}
inline uint32 NetworkPredictComponent::GetFrameActionIDSnap() const
{
    return frameActionID.pureId;
}

inline bool NetworkPredictComponent::IsPredictedComponent(const Type* componentType) const
{
    return (predictedComponentMask & ComponentUtils::MakeMask(componentType)).any();
}

inline void NetworkPredictComponent::AddPredictedComponent(const Type* componentType)
{
    predictedComponentMask |= ComponentUtils::MakeMask(componentType);
}

inline void NetworkPredictComponent::RemovePredictedComponent(const Type* componentType)
{
    predictedComponentMask &= ~ComponentUtils::MakeMask(componentType);
}

} // namespace DAVA

namespace std
{
template <>
struct hash<DAVA::FrameActionID>
{
    std::size_t operator()(const DAVA::FrameActionID& id) const
    {
        return std::hash<DAVA::uint32>()((id.pureId));
    }
};
};
