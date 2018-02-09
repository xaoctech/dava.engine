#pragma once

#include "Base/Map.h"
#include "Base/Vector.h"
#include "Base/FastName.h"

namespace DAVA
{
namespace TrafficLoggerDetail
{
static const float32 TRAFFIC_LOGGER_DURATION = 10.f;
}

class TrafficLogger
{
public:
    enum MeasuredIds
    {
        P50,
        P75,
        P95,
        MAX,
        VOLUME_RATE,
        PACKET_RATE,
        COUNT_IDS
    };

    TrafficLogger(float32 duration = TrafficLoggerDetail::TRAFFIC_LOGGER_DURATION);

    void LogSending(size_t size, uint8 channel);
    void LogReceiving(size_t size, uint8 channel);
    void Flush();

private:
    float32 duration;
    float32 elapsedSec;
    int64 lastUpdateTime;

    Map<uint8, Vector<size_t>> sndVolumes;
    Map<uint8, uint32> sndPackets;

    Map<uint8, Vector<size_t>> rcvVolumes;
    Map<uint8, uint32> rcvPackets;

    Map<uint8, Vector<size_t>> sndResults;
    Map<uint8, Vector<size_t>> rcvResults;

    template <typename V, typename P>
    void Log(size_t size, uint8 channel, V& volumes, P& packets);

    template <typename V, typename P, typename R>
    void CalcResult(V& volumes, P& packets, R& results);

    template <typename R>
    void PrintResults(const FastName& label, const R& results);
};
}
