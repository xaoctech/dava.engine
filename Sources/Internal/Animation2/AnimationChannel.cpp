#include "AnimationChannel.h"

AnimationChannel::AnimationChannel()
{
}

void AnimationChannel::Bind(const DAVA::uint8* data)
{
    this->data = data;
}

void AnimationChannel::Reset(State* state) const
{
    state->time = 0;
}

void AnimationChannel::Advance(State* state) const
{
}
