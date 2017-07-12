#include "AnimationTrack.h"
#include "AnimationChannel.h"

#include "Base/FastName.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
AnimationTrack::State::State(uint32 channelCount)
{
    channelStates.resize(channelCount);
}

const float32* AnimationTrack::State::GetChannelStateValue(uint32 channel) const
{
    DVASSERT(channel < uint32(channelStates.size()));
    return channelStates[channel].GetChannelValue();
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

                uint32 boundData = channels[c].channel.Bind(dataptr);
                if (boundData == 0)
                {
                    channels.clear();
                    return 0;
                }

                dataptr += boundData;
            }
        }
    }

    return uint32(dataptr - _data);
}

void AnimationTrack::Reset(State* state) const
{
    for (uint32 c = 0; c < GetChannelsCount(); ++c)
    {
        channels[c].channel.Reset(&state->channelStates[c]);
    }
}

void AnimationTrack::Advance(float32 dTime, State* state) const
{
    for (uint32 c = 0; c < GetChannelsCount(); ++c)
    {
        channels[c].channel.Advance(dTime, &state->channelStates[c]);
    }
}

const float32* AnimationTrack::GetStateValue(const State* state, uint32 channel) const
{
    DVASSERT(channel < uint32(state->channelStates.size()));
    return state->channelStates[channel].GetChannelValue();
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