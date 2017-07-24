#pragma once

#include "AnimationChannel.h"

namespace DAVA
{
class AnimationTrack
{
public:
    static const uint32 ANIMATION_TRACK_DATA_SIGNATURE = DAVA_MAKEFOURCC('D', 'V', 'A', 'T');

    enum eChannelTarget : uint8
    {
        CHANNEL_TARGET_POSITION = 0,
        CHANNEL_TARGET_ORIENTATION,
        CHANNEL_TARGET_SCALE,

        CHANNEL_TARGET_COUNT
    };

    class State
    {
    public:
        State(uint32 channelCount = 0);

        const float32* GetChannelStateValue(uint32 channel) const;

    private:
        Vector<AnimationChannel::State> channelStates;

        friend class AnimationTrack;
    };

    AnimationTrack() = default;
    ~AnimationTrack() = default;

    uint32 Bind(const uint8* data);

    uint32 GetChannelsCount() const;
    eChannelTarget GetChannelTarget(uint32 channel) const;

    void Evaluate(float32 time, State* state) const;

    const float32* GetStateValue(const State* state, uint32 channel) const;

private:
    struct Channel
    {
        AnimationChannel channel;
        eChannelTarget target;
    };
    Vector<Channel> channels;
};
}
