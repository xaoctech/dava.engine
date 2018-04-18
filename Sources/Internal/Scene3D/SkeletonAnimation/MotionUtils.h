#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class MotionComponent;

struct MotionUtils final
{
    static void UpdateMotionLayers(MotionComponent* motionComponent, float32 timeDelta);
};
}