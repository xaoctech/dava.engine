#include "NetworkStatisticsSingleComponent.h"

#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <Reflection/ReflectionRegistrator.h>

#include <algorithm>

#include <Logger/Logger.h>
#include <Time/SystemTimer.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkStatisticsSingleComponent)
{
    ReflectionRegistrator<NetworkStatisticsSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

NetworkStatisticsSingleComponent::NetworkStatisticsSingleComponent()
    : latencies(NetworkStatisticsSingleComponentDetail::MEASUREMENTS_NUM, 0)
    , offsets(NetworkStatisticsSingleComponentDetail::MEASUREMENTS_NUM, 0)
{
}

void NetworkStatisticsSingleComponent::StartFrameRTTMeasurement(uint32 frameId)
{
    using namespace NetworkStatisticsSingleComponentDetail;
    if (IsExecTime(frameId))
    {
        if (startFrameId == 0)
        {
            startFrameId = frameId;
        }
        uint32 statId = (frameId - startFrameId) / GRANULARITY;
        statId %= MEASUREMENTS_NUM;
        latencies[statId] = SystemTimer::GetMs();
    }
}

void NetworkStatisticsSingleComponent::FlushFrameRTTMeasurement(uint32 frameId)
{
    using namespace NetworkStatisticsSingleComponentDetail;
    if (IsExecTime(frameId))
    {
        uint32 statId = (frameId - startFrameId) / GRANULARITY;
        statId %= MEASUREMENTS_NUM;
        latencies[statId] = SystemTimer::GetMs() - latencies[statId];

        if (statId == 0)
        {
            std::for_each(latencies.begin(), latencies.end(), [](int64& value) {
                const int64 threshold = 5000; // 5 sec
                if (value > threshold)
                {
                    value = 0;
                }
            });
            std::sort(latencies.begin(), latencies.end());
            uint32 p50 = static_cast<uint32>(latencies.size()) / 2;
            uint32 p75 = static_cast<uint32>(latencies.size()) * 75 / 100;
            uint32 p95 = static_cast<uint32>(latencies.size()) * 95 / 100;

            Logger::Debug("[NetworkStatisticsSingleComponent] %u Latencies:", frameId);
            Logger::Debug("    p50: %lld ms", latencies[p50]);
            Logger::Debug("    p75: %lld ms", latencies[p75]);
            Logger::Debug("    p95: %lld ms", latencies[p95]);

            Vector<int64> tmp(MEASUREMENTS_NUM, 0);
            latencies.swap(tmp);
        }
    }
}

void NetworkStatisticsSingleComponent::FlushFrameOffsetMeasurement(uint32 frameId, int32 offset)
{
    using namespace NetworkStatisticsSingleComponentDetail;
    if (IsExecTime(frameId))
    {
        uint32 statId = (frameId - startFrameId) / GRANULARITY;
        statId %= MEASUREMENTS_NUM;
        offsets[statId] = offset;
        if (statId == 0)
        {
            std::sort(offsets.begin(), offsets.end());
            uint32 p50 = static_cast<uint32>(offsets.size()) / 2;
            uint32 p75 = static_cast<uint32>(offsets.size()) * 75 / 100;
            uint32 p95 = static_cast<uint32>(offsets.size()) * 95 / 100;

            Logger::Debug("%u Frame offsets:", frameId);
            Logger::Debug("    p50: %d, ~%lld ms", offsets[p50], offsets[p50] * NetworkTimeSingleComponent::FrameDurationMs);
            Logger::Debug("    p75: %d, ~%lld ms", offsets[p75], offsets[p75] * NetworkTimeSingleComponent::FrameDurationMs);
            Logger::Debug("    p95: %d, ~%lld ms", offsets[p95], offsets[p95] * NetworkTimeSingleComponent::FrameDurationMs);

            Vector<int64> tmp(MEASUREMENTS_NUM, 0);
            latencies.swap(tmp);
        }
    }
}

void NetworkStatisticsSingleComponent::AddTimestamps(uint64 key, std::unique_ptr<NetStatTimestamps> netStatTimestamps)
{
    timestamps.emplace(key, std::move(netStatTimestamps));
}

NetStatTimestamps* NetworkStatisticsSingleComponent::GetTimestamps(uint64 key)
{
    NetStatTimestamps* result = nullptr;
    auto findIt = timestamps.find(key);
    if (findIt != timestamps.end())
    {
        result = findIt->second.get();
    }
    return result;
}

std::unique_ptr<NetStatTimestamps> NetworkStatisticsSingleComponent::RemoveTimestamps(uint64 key)
{
    auto findIt = timestamps.find(key);
    if (findIt != timestamps.end())
    {
        std::unique_ptr<NetStatTimestamps> result = std::move(findIt->second);
        timestamps.erase(key);
        return result;
    }
    return nullptr;
}

void NetworkStatisticsSingleComponent::FlushTimestamps(const NetStatTimestamps* timestamps)
{
    int64 toServerDiff = static_cast<int64>(timestamps->server.recvFromNet) -
    static_cast<int64>(timestamps->client.sendToNet);
    int64 toClientDiff = SystemTimer::GetMs() - static_cast<int64>(timestamps->server.sendToNet);
    int64 rtt = toServerDiff + toClientDiff;
    Logger::Debug("Timestamps: InputToSend=%lld, RecvToFrame=%lld, InsideBuf=%lld, BufToSend=%lld, RTT=%lld",
                  timestamps->client.sendToNet - timestamps->client.input,
                  timestamps->server.newFrame - timestamps->server.recvFromNet,
                  timestamps->server.getFromBuf - timestamps->server.newFrame,
                  timestamps->server.sendToNet - timestamps->server.getFromBuf,
                  rtt);
}

void NetworkStatisticsSingleComponent::UpdateFrameTimestamps()
{
    int64 currTs = SystemTimer::GetMs();
    for (auto& pair : timestamps)
    {
        if (pair.second->server.newFrame == 0)
        {
            pair.second->server.newFrame = static_cast<uint32>(currTs);
        }
    }
}

bool NetworkStatisticsSingleComponent::IsExecTime(uint32 frameId) const
{
    return frameId > 0 && frameId % NetworkStatisticsSingleComponentDetail::GRANULARITY == 0;
}

void NetworkStatisticsSingleComponent::Clear()
{
}

uint64 NetStatTimestamps::GetKey(uint32 frameId, NetworkPlayerID playerID /*=0*/)
{
    return (static_cast<uint64>(playerID) << 32) + frameId;
}

} //namespace DAVA
