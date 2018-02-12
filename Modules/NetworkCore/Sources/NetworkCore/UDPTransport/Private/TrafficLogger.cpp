#include "TrafficLogger.h"
#include "ENetUtils.h"
#include "Logger/Logger.h"
#include "Time/SystemTimer.h"

#include <algorithm>

namespace DAVA
{
TrafficLogger::TrafficLogger(float32 duration /*=TrafficLoggerDetail::TRAFFIC_LOGGER_DURATION*/)
    : duration(duration)
    , elapsedSec(duration)
    , lastUpdateTime(SystemTimer::GetMs())
{
}

template <typename V, typename P>
void TrafficLogger::Log(size_t size, uint8 channel, V& volumes, P& packets)
{
    volumes[channel].push_back(size);
    packets[channel] += 1;
    Flush();
}

void TrafficLogger::LogSending(size_t size, uint8 channel)
{
    Log(size, channel, sndVolumes, sndPackets);
}

void TrafficLogger::LogReceiving(size_t size, uint8 channel)
{
    Log(size, channel, rcvVolumes, rcvPackets);
}

template <typename V, typename P, typename R>
void TrafficLogger::CalcResult(V& volumes, P& packets, R& results)
{
    results.clear();
    for (auto& volumeData : volumes)
    {
        auto& volume = volumeData.second;
        std::sort(volume.begin(), volume.end());
        size_t size = volume.size();
        size_t total = 0;
        std::for_each(volume.begin(), volume.end(), [&total](size_t size) { total += size; });
        results.emplace(volumeData.first,
                        Vector<size_t>{
                        volume[size / 2],
                        volume[size * 3 / 4],
                        volume[size * 95 / 100],
                        volume.back(),
                        static_cast<size_t>(std::round(total / elapsedSec)),
                        static_cast<size_t>(std::round(packets[volumeData.first] / elapsedSec))
                        });
    }
    volumes.clear();
    packets.clear();
}

template <typename R>
void TrafficLogger::PrintResults(const FastName& label, const R& results)
{
    for (const auto& result : results)
    {
        const auto& resultData = result.second;
        Logger::Debug("    [%s] %s p50/p75/p95/max: %u/%u/%u/%u, %u Bytes/s, %u Packets/s",
                      label.c_str(), PacketParams::GetStrChannel(result.first),
                      resultData[P50], resultData[P75], resultData[P95], resultData[MAX],
                      resultData[VOLUME_RATE], resultData[PACKET_RATE]);
    }
}

void TrafficLogger::Flush()
{
    int64 currTime = SystemTimer::GetMs();
    float32 timeElapsed = static_cast<float32>(currTime - lastUpdateTime) / 1000.f;
    elapsedSec += timeElapsed;
    if (elapsedSec >= duration)
    {
        CalcResult(sndVolumes, sndPackets, sndResults);
        CalcResult(rcvVolumes, rcvPackets, rcvResults);
        Logger::Debug("[TrafficLogger] Traffic by channels:");
        PrintResults(FastName("send"), sndResults);
        PrintResults(FastName("recv"), rcvResults);
        elapsedSec = 0.f;
    }
    lastUpdateTime = currTime;
}
}
