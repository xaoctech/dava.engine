#pragma once

#include "Base/BaseTypes.h"
#include "Math/Quaternion.h"
#include "NetworkCore/Private/NetworkSerialization.h"

namespace DAVA
{
class NetworkIdSystem;

using NetworkPlayerID = uint8;
struct NetworkID
{
    SERIALIZABLE(fullValue);

    static const NetworkID INVALID;
    static const NetworkID SCENE_ID;

    NetworkID();

    explicit NetworkID(uint32 value);

    bool IsValid() const;
    bool IsStaticId() const;
    bool IsPlayerOwnId() const;
    bool IsPlayerActionId() const;
    bool IsPlayerId() const;

    NetworkPlayerID GetPlayerId() const;
    uint32 GetPlayerActionFrameId() const;
    uint32 GetPlayerActionId() const;

    explicit operator uint32() const;

    bool operator==(const NetworkID& other) const;
    bool operator!=(const NetworkID& other) const;
    bool operator>=(const NetworkID& other) const;
    bool operator<(const NetworkID& other) const;
    bool operator>(const NetworkID& other) const;

    friend std::ostream& operator<<(std::ostream& stream, const NetworkID& id);

    static NetworkID CreatePlayerActionId(NetworkPlayerID playerId_7bits, uint32 frameId_20bits, uint32 id_4bits);
    static NetworkID CreatePlayerOwnId(NetworkPlayerID playerId_7bits);
    static NetworkID CreateStaticId(uint32 id_30bits);

private:
    union
    {
        uint32 fullValue;

        struct
        {
            uint32 id : 30;
            uint32 playerFlag : 1; // must be 0
            uint32 actionFlag : 1; // must be 0
        } staticValue;

        struct
        {
            uint32 id : 23;
            uint32 playerId : 7;
            uint32 playerFlag : 1; // must be 1
            uint32 actionFlag : 1; // must be 0
        } playerValue;

        struct
        {
            uint32 id : 4;
            uint32 frameId : 20;
            uint32 playerId : 7;
            uint32 actionFlag : 1; // must be 1
        } actionValue;
    };
};

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
    };

    union
    {
        int32 diff;
        float32 frequencyHz;
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

#define __Dava_Network_NetworkType__
#include "NetworkCore/Private/NetworkTypes.hpp"
