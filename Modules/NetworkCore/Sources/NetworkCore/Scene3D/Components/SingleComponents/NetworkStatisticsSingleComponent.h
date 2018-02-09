#pragma once

#include <Entity/SingletonComponent.h>
#include <NetworkCore/NetworkTypes.h>
#include <NetworkCore/Private/NetworkSerialization.h>

namespace DAVA
{
namespace NetworkStatisticsSingleComponentDetail
{
static const uint32 GRANULARITY = 5;
static const uint32 MEASUREMENTS_NUM = 120;
static const uint32 TIMESTAMPS_DELAY = 300;
}

#pragma pack(push, 1)
struct NetStatTimestamps
{
    SERIALIZABLE(client, server)
    struct
    {
        SERIALIZABLE(input, sendToNet)
        uint32 input = 0;
        uint32 sendToNet = 0;
    } client = {};
    struct
    {
        SERIALIZABLE(recvFromNet, newFrame, getFromBuf, sendToNet)
        uint32 recvFromNet = 0;
        uint32 newFrame = 0;
        uint32 getFromBuf = 0;
        uint32 sendToNet = 0;
    } server = {};

    static uint64 GetKey(uint32 frameId, NetworkPlayerID playerID = 0);
};
#pragma pack(pop)

class NetworkStatisticsSingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkStatisticsSingleComponent, SingletonComponent);

    NetworkStatisticsSingleComponent();

    void StartFrameRTTMeasurement(uint32 frameId);
    void FlushFrameRTTMeasurement(uint32 frameId);

    void FlushFrameOffsetMeasurement(uint32 frameId, int32 offset);

    void AddTimestamps(uint64 key, std::unique_ptr<NetStatTimestamps> netStatTimestamps);
    NetStatTimestamps* GetTimestamps(uint64 key);
    std::unique_ptr<NetStatTimestamps> RemoveTimestamps(uint64 key);
    void FlushTimestamps(const NetStatTimestamps* timestamps);
    void UpdateFrameTimestamps();

private:
    uint32 startFrameId = 0;
    Vector<int64> latencies;
    Vector<int32> offsets;
    UnorderedMap<uint64, std::unique_ptr<NetStatTimestamps>> timestamps;

    bool IsExecTime(uint32 frameId) const;
};
}
