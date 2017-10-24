#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
struct UIFlowTransitionEffectConfig final
{
    enum Effect : int32
    {
        None = 0,
        Static,
        FadeAlpha,
        Fade,
        Scale,
        Flip,
        MoveLeft,
        MoveRight
    };

    Effect effectOut = Effect::None;
    Effect effectIn = Effect::None;
    float32 duration = 0.f;
};
}