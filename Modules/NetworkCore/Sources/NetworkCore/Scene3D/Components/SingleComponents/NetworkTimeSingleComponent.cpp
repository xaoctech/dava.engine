#include "NetworkTimeSingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
uint32 NetworkTimeSingleComponent::FrameFrequencyHz = 50;
float32 NetworkTimeSingleComponent::FrameDurationS = 1.f / static_cast<float32>(NetworkTimeSingleComponent::FrameFrequencyHz);
uint32 NetworkTimeSingleComponent::FrameDurationMs = 1000 / NetworkTimeSingleComponent::FrameFrequencyHz;
uint32 NetworkTimeSingleComponent::FrameDurationUs = 1000000 / NetworkTimeSingleComponent::FrameFrequencyHz;
float32 NetworkTimeSingleComponent::FrameSpeedupS = 0.001f;
float32 NetworkTimeSingleComponent::FrameSlowdownS = 0.001f;
float32 NetworkTimeSingleComponent::UptimeInitFactor = 0.5f;
float32 NetworkTimeSingleComponent::UptimeDelayFactor = 3.0f;
int32 NetworkTimeSingleComponent::ArtificialLatency = 4;
float32 NetworkTimeSingleComponent::LossFactor = 0.05f;

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkTimeSingleComponent)
{
    ReflectionRegistrator<NetworkTimeSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

bool NetworkTimeSingleComponent::IsInitialized() const
{
    return initialized;
}
void NetworkTimeSingleComponent::SetIsInitialized(bool value)
{
    initialized = value;
}

uint32 NetworkTimeSingleComponent::GetUptimeMs() const
{
    return uptime;
}

void NetworkTimeSingleComponent::SetUptimeMs(uint32 value)
{
    uptime = value;
}

uint32 NetworkTimeSingleComponent::GetFrameId() const
{
    return frameId;
}

void NetworkTimeSingleComponent::SetFrameId(uint32 value)
{
    frameId = value;
}

int32 NetworkTimeSingleComponent::GetAdjustedFrames() const
{
    return adjustedFrames;
}

void NetworkTimeSingleComponent::SetAdjustedFrames(int32 value)
{
    adjustedFrames = value;
}

int32 NetworkTimeSingleComponent::GetLastSyncDiff() const
{
    return lastSyncDiff;
}

void NetworkTimeSingleComponent::SetLastSyncDiff(int32 value)
{
    lastSyncDiff = value;
}

uint32 NetworkTimeSingleComponent::GetLastServerFrameId() const
{
    return lastServerFrameId;
}

void NetworkTimeSingleComponent::SetLastServerFrameId(uint32 frameId)
{
    lastServerFrameId = frameId;
}

float32 NetworkTimeSingleComponent::GetFps() const
{
    return fps;
}

void NetworkTimeSingleComponent::SetFps(float32 value)
{
    fps = value;
}

void NetworkTimeSingleComponent::SetResyncDelayUptime(const FastName& token, uint32 uptime)
{
    resyncDelayUptime[token] = uptime;
}

uint32 NetworkTimeSingleComponent::GetResyncDelayUptime(const FastName& token) const
{
    uint32 uptime = 0;
    auto findIt = resyncDelayUptime.find(token);
    if (findIt != resyncDelayUptime.end())
    {
        uptime = findIt->second;
    }
    return uptime;
}

void NetworkTimeSingleComponent::SetLastClientFrameId(const FastName& token, uint32 frameId)
{
    lastClientFrameId[token] = frameId;
}

uint32 NetworkTimeSingleComponent::GetLastClientFrameId(const FastName& token) const
{
    uint32 frameID = 0;
    auto findIt = lastClientFrameId.find(token);
    if (findIt != lastClientFrameId.end())
    {
        frameID = findIt->second;
    }
    return frameID;
}

void NetworkTimeSingleComponent::SetClientOutrunning(const FastName& token, int32 diff)
{
    clientOutrunning[token] = diff;
}

int32 NetworkTimeSingleComponent::GetClientOutrunning(const FastName& token) const
{
    int32 diff = 0;
    auto findIt = clientOutrunning.find(token);
    if (findIt != clientOutrunning.end())
    {
        diff = findIt->second;
    }
    return diff;
}

void NetworkTimeSingleComponent::SetClientViewDelay(const FastName& token, uint32 frameID,
                                                    int32 diff)
{
    auto findIt = clientViewDelayHistory.find(token);
    if (findIt != clientViewDelayHistory.end())
    {
        findIt->second[frameID % diffHistorySize] = diff;
    }
    else
    {
        Vector<int32> history(diffHistorySize, 0);
        history[frameID % diffHistorySize] = diff;
        clientViewDelayHistory[token] = history;
    }
}

int32 NetworkTimeSingleComponent::GetClientViewDelay(const FastName& token, uint32 frameID) const
{
    int32 diff = 0;
    auto findIt = clientViewDelayHistory.find(token);
    if (findIt != clientViewDelayHistory.end())
    {
        diff = findIt->second[frameID % diffHistorySize];
    }
    return diff;
}
void NetworkTimeSingleComponent::SetNumTimeSyncs(uint32 value)
{
    numTimeSyncs = value;
}

uint32 NetworkTimeSingleComponent::GetNumTimeSyncs() const
{
    return numTimeSyncs;
}
}
