#pragma once

#include <Entity/SceneSystem.h>
#include <Base/BaseTypes.h>

#include "NetworkCore/Private/NetworkSerialization.h"
#include "NetworkCore/UDPTransport/Private/ENetUtils.h"
#include "NetworkCore/Snapshot.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkStatisticsSingleComponent.h"

namespace DAVA
{
class NetworkDeltaReplicationSystemBase : public SceneSystem
{
protected:
    using SequenceId = uint16;
    struct PreAllocatedBlock
    {
        static const uint32 SIZE = PacketParams::MAX_PACKET_SIZE;
        uint32 size = 0;
        uint8 buff[SIZE] = {};
    };

    struct PacketHeader
    {
        bool isDirty = false;
        std::unique_ptr<NetStatTimestamps> timestamps = nullptr;
        SequenceId sequenceId = 0;
        uint32 frameId = 0;
        uint8 allPartsCount = 0;
        SERIALIZABLE(timestamps, sequenceId, frameId, allPartsCount);

        void Reset()
        {
            isDirty = false;
            timestamps.reset();
            sequenceId = 0;
            frameId = 0;
            allPartsCount = 0;
        }
    };

    struct EntityHeader
    {
        NetworkID netEntityId;
        uint8 frameOffset = 0;
        SERIALIZABLE(netEntityId, frameOffset);
    };

    struct AckPacket
    {
        Vector<SequenceId> sequenceIds;
        inline uint32 Save(uint8* out) const
        {
            uint32 offset = 0;
            for (SequenceId seqId : sequenceIds)
            {
                offset += NetworkSerialization::Save(out + offset, seqId);
            }

            return offset;
        }

        inline uint32 Load(const uint8* in, size_t size)
        {
            sequenceIds.clear();
            uint32 offset = 0;
            while (offset < size)
            {
                SequenceId seqId;
                offset += NetworkSerialization::Load(in + offset, seqId);
                sequenceIds.push_back(seqId);
            }

            return offset;
        }
    };

    PreAllocatedBlock mtuBlock = {};
    NetworkDeltaReplicationSystemBase(Scene* scene);

public:
    DAVA_VIRTUAL_REFLECTION(NetworkDeltaReplicationSystemBase, SceneSystem);

    void PrepareForRemove() override{};

protected:
    //    TODO: Not implement nack fail entities in snapshot system.
    //    UnorderedMap<uint32, UnorderedSet<uint32>> seqToNackEntities;
    AckPacket ackPacket;
};

} //namespace DAVA
