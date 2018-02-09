#pragma once

#include <enet/enet.h>

#include "Base/BaseTypes.h"
#include "Base/Exception.h"

namespace DAVA
{
enum NetworkErrors
{
    NOT_CONNECTED,
    LOOP_ERROR,
};

template <typename T>
void ThrowIfENetError(const T ptr, const std::string& error)
{
    if (nullptr == ptr)
    {
        DAVA_THROW(DAVA::Exception, error.c_str());
    }
};

template <>
void ThrowIfENetError<int>(const int retCode, const std::string& error);

struct PacketParams
{
    static const uint32 MAX_PACKET_SIZE = 4096; // 4KB

    enum Channels : uint8
    {
        SERVICE_CHANNEL_ID = 0,
        PRIVATE_CHANNEL_ID,
        DEFAULT_CHANNEL_ID,
        REPLICATION_CHANNEL_ID,
        REPLICATION_DIFF_CHANNEL_ID,
        INPUT_CHANNEL_ID,
        TRANSPORT_CHANNEL_ID,
        TOKEN_CHANNEL_ID,
        TIME_CHANNEL_ID,
        GAMEMODE_CHANNEL_ID,
        DELTA_REPLICATION_CHANNEL_ID,
        CHANNELS_COUNT
    };

    static const char* GetStrChannel(uint8 channel)
    {
        const char* channelsToStr[] = {
            "SERVICE_CHANNEL",
            "PRIVATE_CHANNEL",
            "DEFAULT_CHANNEL",
            "REPLICATION_CHANNEL",
            "REPLICATION_DIFF_CHANNEL",
            "INPUT_CHANNEL",
            "TRANSPORT_CHANNEL",
            "TOKEN_CHANNEL",
            "TIME_CHANNEL",
            "GAMEMODE_CHANNEL",
            "DELTA_REPLICATION_CHANNEL"
        };
        return channelsToStr[channel];
    }

    static PacketParams Reliable(uint8 channelID = DEFAULT_CHANNEL_ID);
    static PacketParams Unreliable(uint8 channelID = DEFAULT_CHANNEL_ID);

    const uint32 BuildFlags() const;

    uint8 channelID = DEFAULT_CHANNEL_ID;

    bool isReliable = true;
    bool isSequenced = true;
    bool isReliableFragment = true;
    bool isCopy = true;
};

void CompressWithZlib(const uint8* in, size_t insize, uint8* out, size_t& outsize);
void DecompressWithZlib(const uint8* in, size_t insize, uint8* out, size_t& outsize);

bool CompressWithLz4(const uint8* in, size_t insize, uint8* out, size_t& outsize);
bool DecompressWithLz4(const uint8* in, size_t insize, uint8* out, size_t& outsize);

uint32 GetMaxCompressedSizeWithLz4(uint32 size);

float32 GetPacketLoss(ENetPeer* peer, float32& cachedPacketLosses);
} // namespace DAVA
