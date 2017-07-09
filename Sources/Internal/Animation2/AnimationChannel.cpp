#include "AnimationChannel.h"

namespace DAVA
{
AnimationChannel::AnimationChannel()
{
}

uint32 AnimationChannel::Bind(const uint8* _data)
{
    DVASSERT(_data == nullptr || *reinterpret_cast<const uint32*>(_data) == ANIMATION_CHANNEL_DATA_SIGNATURE);

    dimension = keyStride = keysCount = 0;
    data = nullptr;

    if (_data != nullptr)
    {
        const uint8* dataptr = _data;

        dataptr += 4; //skip signature

        dimension = uint32(*dataptr);
        dataptr += 1;

        dataptr += 3; //pad

        keysCount = *reinterpret_cast<const uint32*>(dataptr);
        dataptr += 4;

        data = dataptr;

        keyStride = uint32(sizeof(float32)) * (dimension + 1);
    }

    return uint32(data - _data) + keysCount * keyStride;
}

void AnimationChannel::Reset(State* state) const
{
    state->time = 0;
}

void AnimationChannel::Advance(State* state) const
{
}
}
