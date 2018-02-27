#include "Scene3D/EntityMatcher.h"

#include "Base/TypeInheritance.h"

namespace DAVA
{
bool AllOfEntityMatcher::MatchMask(const ComponentMask& mask1, const ComponentMask& mask2)
{
    return (mask1 & mask2) == mask1;
}

bool AnyOfEntityMatcher::MatchMask(const ComponentMask& mask1, const ComponentMask& mask2)
{
    return (mask1 & mask2) != 0;
}

bool ExactTypeMatcher::MatchType(const Type* type1, const Type* type2)
{
    return type1 == type2;
}

bool BaseOfTypeMatcher::MatchType(const Type* type1, const Type* type2)
{
    return type2->GetInheritance()->CanDownCast(type2, type1);
}
}
