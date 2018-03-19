
#include "NetworkDeltaReplicationSystemClient.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkDeltaSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientConnectionSingleComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkReplicationSystem2.h"
#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/SnapshotUtils.h>

#include "NetworkCore/UDPTransport/UDPClient.h"

#include <Debug/ProfilerCPU.h>
#include <Logger/Logger.h>
#include <Math/MathHelpers.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

#include <algorithm>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkDeltaReplicationSystemClient)
{
    ReflectionRegistrator<NetworkDeltaReplicationSystemClient>::Begin()[M::Tags("network", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkDeltaReplicationSystemClient::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 12.0f)]
    .End();
}

ElasticBuffer::ElasticBuffer()
    : size(PRIMARY_BUFF_SIZE)
    , buff(std::make_unique<uint8[]>(size))
    , idx(0)
{
}

ElasticBuffer::~ElasticBuffer()
{
    if (idx > 0)
    {
        Logger::Warning("Release fallback buffer level:%llu size:%llu", idx, offset);
    }
}

ElasticBuffer::ElasticBuffer(uint32 size, uint32 idx)
    : size(size)
    , buff(std::make_unique<uint8[]>(size))
    , idx(idx)
{
}

void ElasticBuffer::Reset()
{
    offset = 0;
    fallback.reset();
}

const uint8* ElasticBuffer::Insert(const uint8* srcBuff, uint32 srcSize, uint32 align)
{
    DVASSERT(IsPowerOf2(align));

    uint32 dstSize = srcSize;
    if (0 != ((align - 1) & dstSize))
    {
        dstSize &= ~(align - 1);
        dstSize += align;
    }

    if (dstSize <= size - offset)
    {
        uint8* insertPos = buff.get() + offset;
        Memcpy(insertPos, srcBuff, srcSize);
        offset += dstSize;
        DVASSERT(offset <= size);
        return insertPos;
    }

    if (!fallback)
    {
        if (idx >= EXT_PAGE_MAX_COUNT || dstSize > FALLBACK_BUFF_SIZE)
        {
            return nullptr;
        }

        fallback.reset(new ElasticBuffer(FALLBACK_BUFF_SIZE, idx + 1));
    }

    return fallback->Insert(srcBuff, srcSize);
}

uint32 ElasticBuffer::GetOffset() const
{
    return offset;
}

uint32 ElasticBuffer::GetFallbackCount() const
{
    if (fallback)
    {
        return 1 + fallback->GetFallbackCount();
    }
    return 0;
}

const ElasticBuffer& ElasticBuffer::GetTail() const
{
    if (fallback)
    {
        return fallback->GetTail();
    }
    return *this;
}

NetworkDeltaReplicationSystemClient::NetworkDeltaReplicationSystemClient(Scene* scene)
    : NetworkDeltaReplicationSystemBase(scene)
{
    DVASSERT(IsClient(scene));

    client = scene->GetSingleComponentForRead<NetworkClientSingleComponent>(this)->GetClient();

    replicationSingleComponent = scene->GetSingleComponentForWrite<NetworkReplicationSingleComponent>(this);
    deltaSingleComponent = scene->GetSingleComponentForWrite<NetworkDeltaSingleComponent>(this);
    statsComp = scene->GetSingleComponentForWrite<NetworkStatisticsSingleComponent>(this);
    netConnectionComp = scene->GetSingleComponent<NetworkClientConnectionSingleComponent>();
}

void NetworkDeltaReplicationSystemClient::ProcessFixed(float32 timeElapsed)
{
    const auto& recvPackets = netConnectionComp->GetRecvPackets(PacketParams::DELTA_REPLICATION_CHANNEL_ID);
    for (const auto& packet : recvPackets)
    {
        OnReceive(packet);
    }
    ProcessAppliedPackets();
    elasticBuffer.Reset();
}

void NetworkDeltaReplicationSystemClient::OnReceive(const Vector<uint8>& packet)
{
    NetworkDeltaSingleComponent::Deltas& deltas = deltaSingleComponent->deltas;
    PacketHeader pktHeader;
    uint32 size = pktHeader.Load(packet.data());
    const uint32 frameId = pktHeader.frameId;

    if (pktHeader.timestamps)
    {
        DVASSERT(statsComp, "Server sends NetStats but client has not NetworkStatisticsSingleComponent");
        statsComp->FlushTimestamps(pktHeader.timestamps.get());
        statsComp->FlushFrameRTTMeasurement(frameId);
    }

    const SequenceId sequenceId = pktHeader.sequenceId;
    uint32 numberOfEntities = 0;

    UnreliableFragments& fragments = frameToFragments[frameId];
    if (pktHeader.allPartsCount)
    {
        fragments.all = pktHeader.allPartsCount;
    }
    ++fragments.rcv;
    if (fragments.all && fragments.all == fragments.rcv)
    {
        fragments.isEntire = true;
    }

    EntityHeader entHeader;
    const uint32 entHeaderSize = entHeader.GetSize();
    while (size + entHeaderSize <= packet.size())
    {
        size += entHeader.Load(packet.data() + size);
        DVASSERT(size <= packet.size());
        const uint32 baseFrameId = (entHeader.frameOffset > 0) ? frameId - entHeader.frameOffset : 0;
        const uint8* buff = packet.data() + size;

        const size_t tailSize = packet.size() - size;
        size_t diffSize = SnapshotUtils::GetSnapshotDiffSize(buff, tailSize);
        if (!diffSize)
        {
            //DVASSERT(false && "Can't apply incoming diff");
            Logger::Error("Can't apply incoming diff, %u -> %u", baseFrameId, frameId);
            return;
        }

        const uint8* insertPos = elasticBuffer.Insert(buff, static_cast<uint32>(diffSize));
        if (nullptr == insertPos)
        {
            Logger::Error("Buffer overrun size:%llu", diffSize);
            DVASSERT(0);
            return;
        }

        ++numberOfEntities;
        NetworkDeltaSingleComponent::Delta delta;
        delta.sequenceId = sequenceId;
        delta.netEntityId = entHeader.netEntityId;
        delta.baseFrameId = baseFrameId;
        delta.frameId = frameId;
        delta.srcBuff = insertPos;
        delta.srcSize = diffSize;
        deltas.push_back(std::move(delta));

        size += diffSize;
    }

    if (sequenceId && numberOfEntities)
    {
        sequenceToCounter[sequenceId] = numberOfEntities;
    }
}

void NetworkDeltaReplicationSystemClient::ProcessAppliedPackets()
{
    NetworkDeltaSingleComponent::Deltas& deltas = deltaSingleComponent->deltas;
    for (NetworkDeltaSingleComponent::Delta& delta : deltas)
    {
        //DVASSERT(delta.status != NetworkDeltaSingleComponent::Delta::Status::PENDING);
        if (delta.status == NetworkDeltaSingleComponent::Delta::Status::APPLIED)
        {
            if (delta.sequenceId)
            {
                uint32& numberOfEntities = sequenceToCounter[delta.sequenceId];
                if (numberOfEntities > 0)
                {
                    --numberOfEntities;
                    if (delta.sequenceId && 0 == numberOfEntities)
                    {
                        ackPacket.sequenceIds.push_back(delta.sequenceId);
                    }
                }
            }
        }
        else if (delta.status == NetworkDeltaSingleComponent::Delta::Status::SKIPPED)
        {
            frameToFragments.erase(delta.frameId);
        }
    }

    sequenceToCounter.clear();
    if (!ackPacket.sequenceIds.empty())
    {
        PacketParams packetParams = PacketParams::Unreliable(PacketParams::DELTA_REPLICATION_CHANNEL_ID);
        mtuBlock.size = ackPacket.Save(mtuBlock.buff);
        client->Send(mtuBlock.buff, mtuBlock.size, packetParams);
        ackPacket.sequenceIds.clear();
    }

    NetworkReplicationSingleComponent::FullyReceivedFrames& fullyReceivedFrames = replicationSingleComponent->fullyReceivedFrames;
    for (const auto& it : frameToFragments)
    {
        const UnreliableFragments& fragments = it.second;
        if (fragments.isEntire)
        {
            const uint32 frameId = it.first;
            fullyReceivedFrames.insert(frameId);
        }
    }
    frameToFragments.clear();
    const int32 excessCount = static_cast<int32>(fullyReceivedFrames.size() - NetworkReplicationSingleComponent::MAX_ASSEMBLED_FRAMES);
    if (excessCount > 0)
    {
        auto begin = fullyReceivedFrames.begin();
        auto it = begin;
        std::advance(it, std::min(static_cast<size_t>(excessCount), fullyReceivedFrames.size()));
        fullyReceivedFrames.erase(begin, it);
    }
}

} //namespace DAVA
