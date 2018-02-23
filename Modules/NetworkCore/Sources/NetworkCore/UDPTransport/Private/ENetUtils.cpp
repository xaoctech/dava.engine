#include "Logger/Logger.h"
#include "ENetUtils.h"

#include <libpng/zlib.h>
#include <lz4/lz4.h>

namespace DAVA
{
template <>
void ThrowIfENetError<int>(const int retCode, const std::string& error)
{
    if (0 != retCode)
    {
        DAVA_THROW(DAVA::Exception, error.c_str());
    }
}
const uint32 PacketParams::BuildFlags() const
{
    uint32 flags = 0;
    if (isReliable)
    {
        DVASSERT(isSequenced && isReliableFragment);
        flags |= ENET_PACKET_FLAG_RELIABLE;
    }
    else
    {
        if (!isSequenced)
        {
            flags |= ENET_PACKET_FLAG_UNSEQUENCED;
        }
        if (!isReliableFragment)
        {
            flags |= ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
        }
    }
    if (!isCopy)
    {
        flags |= ENET_PACKET_FLAG_NO_ALLOCATE;
    }

    return flags;
};

PacketParams PacketParams::Reliable(uint8 channelID_)
{
    PacketParams params;
    params.channelID = channelID_;
    return params;
};

PacketParams PacketParams::Unreliable(uint8 channelID_)
{
    PacketParams params;
    params.channelID = channelID_;
    params.isReliable = false;
    params.isSequenced = false;
    params.isReliableFragment = false;
    return params;
};

void CompressWithZlib(const uint8* in, size_t insize, uint8* out, size_t& outsize)
{
    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;

    defstream.avail_in = static_cast<uInt>(insize);
    defstream.next_in = (Bytef*)in;
    defstream.avail_out = static_cast<uInt>(outsize);
    defstream.next_out = out;

    deflateInit(&defstream, Z_BEST_COMPRESSION);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);

    outsize = defstream.next_out - out;
}

void DecompressWithZlib(const uint8* in, size_t insize, uint8* out, size_t& outsize)
{
    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;

    infstream.avail_in = static_cast<uInt>(insize);
    infstream.next_in = (Bytef*)in;
    infstream.avail_out = static_cast<uInt>(outsize);
    infstream.next_out = out;

    inflateInit(&infstream);
    inflate(&infstream, Z_NO_FLUSH);
    inflateEnd(&infstream);

    outsize = infstream.next_out - out;
}

bool CompressWithLz4(const uint8* in, size_t insize, uint8* out, size_t& outsize)
{
    int32 result = LZ4_compress((const char*)in, (char*)out, static_cast<int32>(insize));
    DVASSERT(0 != result);
    outsize = result;
    return result > 0;
}

bool DecompressWithLz4(const uint8* in, size_t insize, uint8* out, size_t& outsize)
{
    int32 result = LZ4_decompress_safe((const char*)in, (char*)out, static_cast<int32>(insize), static_cast<int32>(outsize));
    outsize = result;
    return result >= 0;
}

uint32 GetMaxCompressedSizeWithLz4(uint32 size)
{
    return LZ4_compressBound(size);
}

float32 GetPacketLoss(ENetPeer* peer, float32& cachedPacketLosses)
{
    const float32 LOST_PACKETS_WEIGHT = 0.992f;
    if (peer->packetsSent == 0)
    {
        return cachedPacketLosses;
    }
    float32 ratio = static_cast<float32>(peer->packetsLost) / peer->packetsSent;
    cachedPacketLosses = (cachedPacketLosses * LOST_PACKETS_WEIGHT) + ratio * (1.f - LOST_PACKETS_WEIGHT);
    return cachedPacketLosses;
}
} // namespace DAVA
