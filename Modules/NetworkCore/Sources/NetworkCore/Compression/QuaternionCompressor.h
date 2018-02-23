#pragma once

#include "NetworkCore/Compression/Compression.h"

namespace DAVA
{
class BitReader;
class BitWriter;
class ReflectedMeta;
class Quaternion;

class QuaternionCompressor
{
public:
    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(const Quaternion& value1, const Quaternion& value2, float32 comparePrecision);

    static bool CompressDelta(const Quaternion& value1, const Quaternion& value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);
    static void CompressFull(const Quaternion& value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);

    static void DecompressDelta(const Quaternion& baseValue, Quaternion* targetValue, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(Quaternion* targetValue, CompressionScheme scheme, BitReader& reader);
};

} // namespace DAVA
