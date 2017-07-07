#pragma once

#include "AnimationChannel.h"

namespace DAVA
{
class AnimationTrack
{
public:
    static const uint32 ANIMATION_TRACK_DATA_SIGNATURE = DAVA_MAKEFOURCC('D', 'V', 'A', 'T');

    class State;

    AnimationTrack() = default;
    ~AnimationTrack();

    uint32 Bind(const uint8* data);

    uint32 GetChannelsCount() const;
    const char* GetChannelName(uint32 channel) const;

    void Reset(State* state) const;
    void Advance(State* state) const;

    const float32* GetValue(const State& state, uint32 channel) const;

private:
    uint32 channelsCount = 0;
    AnimationChannel* channels = nullptr;
    const char** channelNames = nullptr;
};

class AnimationTrack::State
{
private:
    uint32 stateCount = 0;
    AnimationChannel::State* state = nullptr;

    friend class AnimationTrack;
};
}
