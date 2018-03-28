#pragma once

#ifndef __Dava_Network_NetworkType__
#include "NetworkCore/NetworkTypes.h"
#endif

namespace DAVA
{
inline NetworkID::NetworkID()
{
    *this = INVALID;
}

inline NetworkID::NetworkID(uint32 id)
    : fullValue(id)
{
    // clear union
    static_assert(sizeof(playerValue) == sizeof(fullValue), "fix enum");
    static_assert(sizeof(actionValue) == sizeof(fullValue), "fix enum");
    static_assert(sizeof(staticValue) == sizeof(fullValue), "fix enum");
}

inline NetworkID::operator uint32() const
{
    return fullValue;
}

inline bool NetworkID::operator==(const NetworkID& other) const
{
    return fullValue == other.fullValue;
}

inline bool NetworkID::operator!=(const NetworkID& other) const
{
    return fullValue != other.fullValue;
}

inline bool NetworkID::operator>=(const NetworkID& other) const
{
    return fullValue >= other.fullValue;
}

inline bool NetworkID::operator<(const NetworkID& other) const
{
    return fullValue < other.fullValue;
}

inline bool NetworkID::operator>(const NetworkID& other) const
{
    return fullValue > other.fullValue;
}

inline bool NetworkID::IsPlayerActionId() const
{
    return (fullValue != INVALID.fullValue && actionValue.actionFlag == 1);
}

inline bool NetworkID::IsStaticId() const
{
    return (staticValue.actionFlag == 0 && staticValue.playerFlag == 0);
}

inline bool NetworkID::IsPlayerOwnId() const
{
    return (playerValue.actionFlag == 0 && playerValue.playerFlag == 1);
}

inline bool NetworkID::IsPlayerId() const
{
    return IsValid() && (IsPlayerActionId() || IsPlayerOwnId());
}
} // namespace DAVA

namespace std
{
template <>
struct hash<DAVA::NetworkID>
{
    size_t operator()(const DAVA::NetworkID& networkID) const
    {
        return static_cast<DAVA::uint32>(networkID);
    }
};
} // namespace std
