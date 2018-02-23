#pragma once

#include "NetworkCore/Compression/Compression.h"

namespace DAVA
{
class BitReader;
class BitWriter;
class ReflectedMeta;
class Vector3;

class Vector3Compressor
{
public:
    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(const Vector3& value1, const Vector3& value2, float32 comparePrecision);

    static bool CompressDelta(const Vector3& value1, const Vector3& value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);
    static void CompressFull(const Vector3& value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);

    static void DecompressDelta(const Vector3& baseValue, Vector3* targetValue, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(Vector3* targetValue, CompressionScheme scheme, BitReader& reader);
};

} // namespace DAVA
