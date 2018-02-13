#include "NetworkTimeSingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
uint32 NetworkTimeSingleComponent::FrequencyHz = 60;
float32 NetworkTimeSingleComponent::FrameDurationS = 1.f / 60.f;
uint32 NetworkTimeSingleComponent::FrameDurationMs = 1000 / 60;
uint32 NetworkTimeSingleComponent::FrameDurationUs = 1000000 / 60;
const float32 NetworkTimeSingleComponent::FrameAccelerationS = 0.001f;

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

int32 NetworkTimeSingleComponent::GetWaitFrames() const
{
    return waitFrames;
}

void NetworkTimeSingleComponent::SetWaitFrames(int32 value)
{
    waitFrames = value;
}

uint32 NetworkTimeSingleComponent::GetLastServerFrameId() const
{
    return lastServerFrameId;
}

void NetworkTimeSingleComponent::SetLastServerFrameId(uint32 value)
{
    lastServerFrameId = value;
}

float32 NetworkTimeSingleComponent::GetFps() const
{
    return fps;
}

void NetworkTimeSingleComponent::SetFps(float32 value)
{
    fps = value;
}

void NetworkTimeSingleComponent::SetLastClientFrameId(const FastName& token, uint32 frameId)
{
    lastClientFrameIds[token] = frameId;
}

uint32 NetworkTimeSingleComponent::GetLastClientFrameId(const FastName& token) const
{
    uint32 lastClientFrameId = 0;
    auto findIt = lastClientFrameIds.find(token);
    if (findIt != lastClientFrameIds.end())
    {
        lastClientFrameId = findIt->second;
    }
    return lastClientFrameId;
}

void NetworkTimeSingleComponent::SetClientServerDiff(const FastName& token, int32 diff)
{
    clientServerDiff[token] = diff;
}

int32 NetworkTimeSingleComponent::GetClientServerDiff(const FastName& token) const
{
    int32 diff = 0;
    auto findIt = clientServerDiff.find(token);
    if (findIt != clientServerDiff.end())
    {
        diff = findIt->second;
    }
    return diff;
}

void NetworkTimeSingleComponent::SetFrequencyHz(float32 freqHz)
{
    FrequencyHz = static_cast<uint32>(freqHz);
    FrameDurationS = 1.f / freqHz;
    FrameDurationMs = 1000 / FrequencyHz;
    FrameDurationUs = 1000000 / FrequencyHz;
}

void NetworkTimeSingleComponent::Clear()
{
}
}
