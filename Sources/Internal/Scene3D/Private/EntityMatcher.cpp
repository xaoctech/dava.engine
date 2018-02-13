#include "Scene3D/EntityMatcher.h"

namespace DAVA
{
bool AllOfEntityMatcher::Match(const ComponentMask& mask1, const ComponentMask& mask2)
{
    return (mask1 & mask2) == mask1;
}

bool AnyOfEntityMatcher::Match(const ComponentMask& mask1, const ComponentMask& mask2)
{
    return (mask1 & mask2) != 0;
}
}
