#pragma once

#include "NetworkCore/Compression/Compression.h"

namespace DAVA
{
class BitReader;
class BitWriter;
class ReflectedMeta;

class FloatCompressor
{
public:
    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(float32 value1, float32 value2, float32 comparePrecision);

    static bool CompressDelta(float32 value1, float32 value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);
    static void CompressFull(float32 value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);

    static void DecompressDelta(float32 baseValue, float32* targetValue, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(float32* targetValue, CompressionScheme scheme, BitReader& reader);
};

} // namespace DAVA
