#pragma once

#include <Entity/SingletonComponent.h>
#include <NetworkCore/NetworkTypes.h>

namespace DAVA
{
class NetworkReplicationSingleComponent : public SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(NetworkReplicationSingleComponent, SingletonComponent);

public:
    static const uint8 MAX_ASSEMBLED_FRAMES = 32;
    struct EntityReplicationInfo
    {
        uint32 frameIdLastTouch = 0;
        uint32 frameIdLastChange = 0;
        uint32 frameIdLastApply = 0;
        uint32 frameIdServer = 0;
    };

    using EntityToInfo = UnorderedMap<NetworkID, EntityReplicationInfo>;
    EntityToInfo replicationInfo;

    using FullyReceivedFrames = Set<uint32>;
    FullyReceivedFrames fullyReceivedFrames;
};
}
