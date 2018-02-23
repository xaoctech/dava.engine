#include "Base/Private/Any_implTypeData.h"

namespace DAVA
{
uint32 AnyTypeData::GetCastMapIndex()
{
    static uint32 index = Type::AllocUserData();
    return index;
}

uint32 AnyTypeData::GetHashFnIndex()
{
    static uint32 index = Type::AllocUserData();
    return index;
}
} // namespace DAVA
