#pragma once

#include <Entity/SceneSystem.h>
#include <Base/BaseTypes.h>

#include "NetworkCore/Private/NetworkSerialization.h"
#include "NetworkCore/UDPTransport/Private/ENetUtils.h"
#include "NetworkCore/Snapshot.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkStatisticsSingleComponent.h"

namespace DAVA
{
class ElasticBuffer
{
public:
    static const uint32 PrimaryBuffSize = PacketParams::MAX_PACKET_SIZE * 512; //2Mb
    static const uint32 FallbackBuffSize = PacketParams::MAX_PACKET_SIZE * 256; //1Mb
    static const uint32 ExtPageMaxCount = 5;

    ElasticBuffer();
    ~ElasticBuffer();
    void Reset();
    const uint8* Insert(const uint8* srcBuff, uint32 srcSize, uint32 align = 4);

    uint32 GetOffset() const;
    uint32 GetFallbackCount() const;
    const ElasticBuffer& GetTail() const;

private:
    explicit ElasticBuffer(uint32 size, uint32 idx);

    uint32 offset = 0;
    uint32 size = 0;
    std::unique_ptr<uint8[]> buff = nullptr;
    std::unique_ptr<ElasticBuffer> fallback = nullptr;
    const uint32 idx = 0;
};

class NetworkDeltaReplicationSystemBase : public SceneSystem
{
protected:
    using SequenceId = uint16;
    static const SequenceId MaxSentCount = 1000;

    struct PreAllocatedBlock
    {
        static const uint32 Size = PacketParams::MAX_PACKET_SIZE;
        uint32 size = 0;
        uint8 buff[Size] = {};
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
