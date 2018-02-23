#pragma once

#include "NetworkCore/NetworkTypes.h"
#include "NetworkCore/Compression/Compression.h"

namespace DAVA
{
class BitReader;
class BitWriter;
class ReflectedMeta;

class NetworkIDCompressor
{
public:
    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(NetworkID value1, NetworkID value2, float32 comparePrecision);

    static bool CompressDelta(NetworkID value1, NetworkID value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);
    static void CompressFull(NetworkID value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);

    static void DecompressDelta(NetworkID baseValue, NetworkID* targetValue, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(NetworkID* targetValue, CompressionScheme scheme, BitReader& reader);
};

} // namespace DAVA
