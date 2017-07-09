#include "AnimationTrack.h"

#include "Base/FastName.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
AnimationTrack::~AnimationTrack()
{
}

uint32 AnimationTrack::Bind(const uint8* _data)
{
    channels.clear();

    const uint8* dataptr = _data;
    if (dataptr && *reinterpret_cast<const uint32*>(dataptr) == ANIMATION_TRACK_DATA_SIGNATURE)
    {
        if (_data != nullptr)
        {
            dataptr += 4; //skip signature

            uint32 channelsCount = *reinterpret_cast<const uint32*>(dataptr);
            dataptr += 4;

            channels.resize(channelsCount);

            for (uint32 c = 0; c < channelsCount; ++c)
            {
                channels[c].target = eChannelTarget(*dataptr);
                dataptr += 1;

                dataptr += 3; //pad

                dataptr += channels[c].channel.Bind(dataptr);
            }
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
    return uint32(channels.size());
}

AnimationTrack::eChannelTarget AnimationTrack::GetChannelTarget(uint32 channel) const
{
    DVASSERT(channel < GetChannelsCount());
    return channels[channel].target;
}
}