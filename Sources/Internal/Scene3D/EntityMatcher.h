#pragma once

#include "Base/Type.h"
#include "Entity/ComponentMask.h"

namespace DAVA
{
using MaskMatcherFunction = bool (*)(const ComponentMask&, const ComponentMask&);
using TypeMatcherFunction = bool (*)(const Type*, const Type*);

class AllOfEntityMatcher
{
public:
    static bool MatchMask(const ComponentMask& mask1, const ComponentMask& mask2);
};

class AnyOfEntityMatcher
{
public:
    static bool MatchMask(const ComponentMask& mask1, const ComponentMask& mask2);
};

class ExactTypeMatcher
{
public:
    static bool MatchType(const Type* type1, const Type* type2);
};

class BaseOfTypeMatcher
{
public:
    static bool MatchType(const Type* type1, const Type* type2);
};
}
