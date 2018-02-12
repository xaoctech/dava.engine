#pragma once

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

namespace DAVA
{
struct PackedCircles
{
    //Circles coords in range [-0.5, 0.5]
    //Circles indexing in single pack sorted at first by 'y' and at second by 'x'

    static const uint32 CIRCLES_PACKINGS_COUNT = 100;
    static const uint32 CIRCLES_PACKINGS_ARRAY_SIZE = CIRCLES_PACKINGS_COUNT * (CIRCLES_PACKINGS_COUNT + 1) / 2;

    static const Array<float32, CIRCLES_PACKINGS_COUNT> CIRCLES_PACKINGS_RADIUSES;
    static const Array<Vector2, CIRCLES_PACKINGS_ARRAY_SIZE> CIRCLES_PACKINGS_COORDS;

    static const uint32 GetCirclesCoordsBaseIndex(uint32 circlesCount);
};

} //ns
