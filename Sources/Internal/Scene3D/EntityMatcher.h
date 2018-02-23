#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
using MatcherFunction = bool (*)(const ComponentMask&, const ComponentMask&);

class AllOfEntityMatcher
{
public:
    static bool Match(const ComponentMask& mask1, const ComponentMask& mask2);
};

class AnyOfEntityMatcher
{
public:
    static bool Match(const ComponentMask& mask1, const ComponentMask& mask2);
};
}