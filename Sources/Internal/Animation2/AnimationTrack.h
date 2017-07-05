#pragma once

#include "AnimationChannel.h"

class AnimationTrack
{
public:
    class State;

    void Bind(const DAVA::uint8* data);

    void Reset(State* state) const;
    void Advance(State* state) const;

    const float* Value(const State& state, unsigned channel_i) const;

private:
};

class AnimationTrack::State
{
public:
    State();
    ~State();

private:
    friend class AnimationTrack;

    unsigned stateCount;
    AnimationChannel::State* state;
};