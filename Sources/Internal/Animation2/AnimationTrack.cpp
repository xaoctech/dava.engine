#include "AnimationTrack.h"
#include "Debug/DVAssert.h"

void AnimationTrack::Bind(const DAVA::uint8* data)
{
}

void AnimationTrack::Reset(State* state) const
{
}

void AnimationTrack::Advance(State* state) const
{
}

const float* AnimationTrack::Value(const State& state, unsigned channel_i) const
{
    DVASSERT(channel_i < state.stateCount);
    return state.state[channel_i].Value();
}
