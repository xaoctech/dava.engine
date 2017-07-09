#include "AnimationTrack.h"

#include "Base/FastName.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
AnimationTrack::~AnimationTrack()
{
    SafeDeleteArray(channels);
    SafeDeleteArray(channelTargets);
}

uint32 AnimationTrack::Bind(const uint8* _data)
{
    DVASSERT(_data == nullptr || *reinterpret_cast<const uint32*>(_data) == ANIMATION_TRACK_DATA_SIGNATURE);

    SafeDeleteArray(channels);
    SafeDeleteArray(channelTargets);

    const uint8* dataptr = _data;
    if (_data != nullptr)
    {
        dataptr += 4; //skip signature

        channelsCount = *reinterpret_cast<const uint32*>(dataptr);
        dataptr += 4;

        channels = new AnimationChannel[channelsCount];
        channelTargets = new eChannelTarget[channelsCount];

        for (uint32 c = 0; c < channelsCount; ++c)
        {
            channelTargets[c] = eChannelTarget(*dataptr);
            dataptr += 1;

            dataptr += 3; //pad

            dataptr += channels[c].Bind(dataptr);
        }
    }

    return uint32(dataptr - _data);
}

void AnimationTrack::Reset(State* state) const
{
}

void AnimationTrack::Advance(State* state) const
{
}

const float32* AnimationTrack::GetValue(const State& state, uint32 channel) const
{
    DVASSERT(channel < state.stateCount);
    return state.state[channel].GetValue();
}

uint32 AnimationTrack::GetChannelsCount() const
{
    return channelsCount;
}

AnimationTrack::eChannelTarget AnimationTrack::GetChannelTarget(uint32 channel) const
{
    DVASSERT(channel < channelsCount);
    return channelTargets[channel];
}
}