#pragma once

#include <Entity/SceneSystem.h>
#include <Base/BaseTypes.h>

#include "NetworkCore/Private/NetworkSerialization.h"
#include "NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemBase.h"

namespace DAVA
{
class IClient;
class NetworkReplicationSingleComponent;
class NetworkDeltaSingleComponent;
class NetworkStatisticsSingleComponent;

class ElasticBuffer
{
public:
    static const uint32 PRIMARY_BUFF_SIZE = PacketParams::MAX_PACKET_SIZE * 512; //2Mb
    static const uint32 FALLBACK_BUFF_SIZE = PacketParams::MAX_PACKET_SIZE * 256; //1Mb
    static const uint32 EXT_PAGE_MAX_COUNT = 5;

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

class NetworkDeltaReplicationSystemClient : public NetworkDeltaReplicationSystemBase
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkDeltaReplicationSystemClient, NetworkDeltaReplicationSystemBase);

    NetworkDeltaReplicationSystemClient(Scene* scene);
    void ProcessFixed(float32 timeElapsed) override;
    void OnReceiveCallback(const uint8* data, size_t size, uint8, uint32);

private:
    void ProcessAppliedPackets();

    IClient* client;
    NetworkReplicationSingleComponent* replicationSingleComponent;
    NetworkDeltaSingleComponent* deltaSingleComponent;
    NetworkStatisticsSingleComponent* statsComp;

    ElasticBuffer elasticBuffer;
    UnorderedMap<SequenceId, uint32> sequenceToCounter;

    struct UnreliableFragments
    {
        uint8 all = 0;
        uint8 rcv = 0;
        bool isEntire = false;
    };

    UnorderedMap<uint32, UnreliableFragments> frameToFragments;
};

} //namespace DAVA
