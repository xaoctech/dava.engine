#pragma once

#include "Base/BaseTypes.h"
#include "Math/Vector.h"
#include "Math/Quaternion.h"
#include "NetworkCore/Private/NetworkSerialization.h"

namespace DAVA
{
struct NetworkID
{
    static const NetworkID INVALID;
    static const NetworkID SCENE_ID;

    static const NetworkID FIRST_STATIC_OBJ_ID;
    static const NetworkID FIRST_SERVER_ID;

    SERIALIZABLE(value);

    NetworkID()
        : value(~0)
    {
    }

    explicit NetworkID(uint32 networkID)
        : value(networkID)
    {
    }

    explicit operator uint32() const
    {
        return value;
    }

    bool operator==(const NetworkID& other) const
    {
        return value == other.value;
    }

    bool operator!=(const NetworkID& other) const
    {
        return value != other.value;
    }

    bool operator<(const NetworkID& other) const
    {
        return value < other.value;
    }

    bool operator>(const NetworkID& other) const
    {
        return value > other.value;
    }

    bool operator>=(const NetworkID& other) const
    {
        return value >= other.value;
    }

    NetworkID& operator++()
    {
        ++value;
        return *this;
    }

    NetworkID operator++(int)
    {
        NetworkID tmp = *this;
        ++*this;
        return tmp;
    }

    friend std::ostream& operator<<(std::ostream& stream, const NetworkID& id)
    {
        stream << static_cast<uint32>(id);
        return stream;
    }

private:
    uint32 value;
};

using NetworkPlayerID = uint8;
constexpr int32 MAX_NETWORK_PLAYERS_COUNT = 128;
constexpr int32 MAX_NETWORK_VISIBLE_ENTITIES_COUNT = 1024;

#pragma pack(push, 1)

struct InputPacketHeader
{
    NetworkID entityId;
    uint32 frameId;
    uint8 framesCount;
    bool hasNetStat;
};
const uint32 INPUT_PACKET_HEADER_SIZE = sizeof(InputPacketHeader);

struct TimeSyncHeader
{
    enum class Type : uint8
    {
        UPTIME,
        DIFF,
        FRAME
    };

    union
    {
        uint32 frameId;
        uint32 uptimeMs;
        int32 diff;
    };

    int8 netDiff;
    Type type;
};
const uint32 TIMESYNC_PACKET_HEADER_SIZE = sizeof(TimeSyncHeader);

struct ServicePacketHeader
{
    enum class ServiceType : uint8
    {
        TIMELINE_CONTROL
    };

    enum class TimelineControlType : uint8
    {
        PAUSE,
        UNPAUSE,
        STEP_OVER
    };

    ServiceType type;
    union
    {
        TimelineControlType timeline;
    } value;
};
const uint32 SERVICE_PACKET_HEADER_SIZE = sizeof(ServicePacketHeader);

struct TokenPacketHeader
{
    static const uint32 TOKEN_LENGTH = 64;
    char8 token[TOKEN_LENGTH];
};

const uint32 PACKET_COUNT_SIZE = sizeof(uint8);

struct GameModePacketHeader
{
    union
    {
        bool isLoaded;
        NetworkPlayerID networkPlayerID;
    };
};
const uint32 GAMEMODE_PACKET_HEADER_SIZE = sizeof(GameModePacketHeader);

struct NetworkPackedQuaternion
{
#ifdef DISABLE_LOSSY_PACK
    Quaternion data;

    bool operator==(const NetworkPackedQuaternion& rhs) const
    {
        return data == rhs.data;
    }
#else
    uint32 lowBytes = 0;
    uint16 highBytes = 0;
    NetworkPackedQuaternion(uint64 pack)
    {
        lowBytes = pack & 0xFFFFFFFF;
        highBytes = (pack >> 32) & 0xFFFF;
    }
    uint64 GetData() const
    {
        return (static_cast<uint64>(highBytes) << 32) | static_cast<uint64>(lowBytes);
    }

    bool operator==(const NetworkPackedQuaternion& rhs) const
    {
        return lowBytes == rhs.lowBytes && highBytes == rhs.highBytes;
    }
#endif
};
#pragma pack(pop)

} //namespace DAVA

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
}
