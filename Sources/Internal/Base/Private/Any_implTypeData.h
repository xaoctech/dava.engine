#pragma once

#ifndef __Dava_Any__
#include "Base/Any.h"
#endif

#include "Base/BaseTypes.h"

namespace DAVA
{
struct AnyTypeData
{
    static uint32 GetCastMapIndex();
    static uint32 GetHashFnIndex();
};
} // namespace DAVA
