#pragma once

#include "AnimationChannel.h"

namespace DAVA
{
class AnimationTrack
{
public:
    static const uint32 ANIMATION_TRACK_DATA_SIGNATURE = DAVA_MAKEFOURCC('D', 'V', 'A', 'T');
    static const char* ANIMATION_CHANNEL_NAME_POSITION;
    static const char* ANIMATION_CHANNEL_NAME_ORIENTATION;
    static const char* ANIMATION_CHANNEL_NAME_SCALE;

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
    AnimationChannel* channels = nullptr;
    const char** channelNames = nullptr;
    uint32 channelsCount = 0;
};

class AnimationTrack::State
{
private:
    uint32 stateCount = 0;
    AnimationChannel::State* state = nullptr;

    friend class AnimationTrack;
};
}
