#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SingletonComponent.h>
#include <Reflection/Reflection.h>
#include <Base/UnordererMap.h>
#include <Base/FastName.h>

namespace DAVA
{
class NetworkTimeSingleComponent : public SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(NetworkTimeSingleComponent, SingletonComponent);

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

    // number of frames with unusual frame duration
    // if waitFrames > 0 then frame duration > constant duration
    // else frame duration < constant duration
    int32 GetWaitFrames() const;
    void SetWaitFrames(int32 value);

    // last approved frame ID from server
    uint32 GetLastServerFrameId() const;
    void SetLastServerFrameId(uint32 value);

    // frames per second
    float32 GetFps() const;
    void SetFps(float32 value);

    // last frame ID was received from client
    void SetLastClientFrameId(const FastName& token, uint32 frameId);
    uint32 GetLastClientFrameId(const FastName& token) const;

    // difference between client and server frame ids
    void SetClientServerDiff(const FastName& token, int32 diff);
    int32 GetClientServerDiff(const FastName& token) const;

    static void SetFrequencyHz(float32 freqHz);

    static uint32 FrequencyHz;
    static float32 FrameDurationS;
    static uint32 FrameDurationMs;
    static uint32 FrameDurationUs;
    static const float32 FrameAccelerationS;

    virtual ~NetworkTimeSingleComponent(){};

private:
    bool initialized = false;
    uint32 uptime = 0;
    uint32 frameId = 0;
    int32 waitFrames = 0;
    uint32 lastServerFrameId = 0;
    float32 fps = 0.f;

    UnorderedMap<FastName, uint32> lastClientFrameIds;
    UnorderedMap<FastName, int32> clientServerDiff;
};
}
