#include "AnimationTrack.h"

#include "Base/FastName.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
const char* AnimationTrack::ANIMATION_CHANNEL_NAME_POSITION = "position";
const char* AnimationTrack::ANIMATION_CHANNEL_NAME_ORIENTATION = "orientation";
const char* AnimationTrack::ANIMATION_CHANNEL_NAME_SCALE = "scale";

AnimationTrack::~AnimationTrack()
{
    SafeDeleteArray(channels);
    SafeDeleteArray(channelNames);
}

uint32 AnimationTrack::Bind(const uint8* _data)
{
    DVASSERT(_data == nullptr || *reinterpret_cast<const uint32*>(_data) == ANIMATION_TRACK_DATA_SIGNATURE);

    SafeDeleteArray(channels);
    SafeDeleteArray(channelNames);

    const uint8* dataptr = _data;
    if (_data != nullptr)
    {
        dataptr += 4; //skip signature

        channelsCount = *reinterpret_cast<const uint32*>(dataptr);
        dataptr += 4;

        channels = new AnimationChannel[channelsCount];
        channelNames = new const char*[channelsCount];

        for (uint32 c = 0; c < channelsCount; ++c)
        {
            channelNames[c] = reinterpret_cast<const char*>(dataptr);
            dataptr += strlen(channelNames[c]) + 1;

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

const char* AnimationTrack::GetChannelName(uint32 channel) const
{
    DVASSERT(channel < channelsCount);
    return channelNames[channel];
}
}