#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SingleComponent.h>
#include <Reflection/Reflection.h>
#include <Base/UnordererMap.h>
#include <Base/FastName.h>

namespace DAVA
{
class NetworkTimeSingleComponent : public SingleComponent
{
    DAVA_VIRTUAL_REFLECTION(NetworkTimeSingleComponent, SingleComponent);

public:
    // client flag to check receiving uptime from server
    bool IsInitialized() const;
    void SetIsInitialized(bool value);

    // server uptime in milliseconds
    uint32 GetUptimeMs() const;
    void SetUptimeMs(uint32 value);

    // server and client frame ID
    uint32 GetFrameId() const;
    void SetFrameId(uint32 value);

    // number of adjusted frames (frames with unusual frame duration)
    // if adjustedFrames > 0 then we slow down: frame duration > constant duration
    // if adjustedFrames < 0 then we speed up: frame duration < constant duration
    int32 GetAdjustedFrames() const;
    void SetAdjustedFrames(int32 value);

    // last frames diff processed during synchronization
    int32 GetLastSyncDiff() const;
    void SetLastSyncDiff(int32 value);

    // last frame ID received from server
    uint32 GetLastServerFrameId() const;
    void SetLastServerFrameId(uint32 frameId);

    // frames per second
    float32 GetFps() const;
    void SetFps(float32 value);

    // delay end uptime after doing a resync
    void SetResyncDelayUptime(const FastName& token, uint32 uptime);
    uint32 GetResyncDelayUptime(const FastName& token) const;

    // last frame ID was received from client
    void SetLastClientFrameId(const FastName& token, uint32 frameId);
    uint32 GetLastClientFrameId(const FastName& token) const;

    // difference between client and server frame ids (positive if client is ahead of server)
    void SetClientOutrunning(const FastName& token, int32 diff);
    int32 GetClientOutrunning(const FastName& token) const;

    // client view delay in frames
    void SetClientViewDelay(const FastName& token, uint32 frameID, int32 diff);
    int32 GetClientViewDelay(const FastName& token, uint32 frameID) const;

    void SetNumTimeSyncs(uint32 value);
    uint32 GetNumTimeSyncs() const;

    virtual ~NetworkTimeSingleComponent(){};

    static float32 FrameFrequencyHz;
    static float32 FrameDurationS;
    static uint32 FrameDurationMs;
    static uint32 FrameDurationUs;
    static float32 FrameSpeedupS; // speedup: time in seconds adjusted frame is decreased by
    static float32 FrameSlowdownS; // slowdown: time in seconds adjusted frame is increased by
    static float32 UptimeInitFactor; // factor applied to RTT when setting client uptime
    static float32 UptimeDelayFactor; // factor applied to RTT when making a delay after setting uptime
    static int32 ArtificialLatency; // difference in frames we try to keep between client and server
    static float32 LossFactor; // frames difference buffer is increased by (packets loss) / LossFactor

private:
    static const uint32 diffHistorySize = 32;

    bool initialized = false;
    uint32 uptime = 0;
    uint32 frameId = 0;
    uint32 lastServerFrameId;
    int32 adjustedFrames = 0;
    int32 lastSyncDiff = 0;
    float32 fps = 0.f;
    uint32 numTimeSyncs = 0;

    UnorderedMap<FastName, uint32> resyncDelayUptime;
    UnorderedMap<FastName, uint32> lastClientFrameId;
    UnorderedMap<FastName, int32> clientOutrunning;
    UnorderedMap<FastName, Vector<int32>> clientViewDelayHistory;
};
}
