#pragma once

#include "NetworkCore/Compression/Compression.h"

namespace DAVA
{
class BitReader;
class BitWriter;
class ReflectedMeta;
class Vector2;

class Vector2Compressor
{
public:
    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(const Vector2& value1, const Vector2& value2, float32 comparePrecision);

    static bool CompressDelta(const Vector2& value1, const Vector2& value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);
    static void CompressFull(const Vector2& value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);

    static void DecompressDelta(const Vector2& baseValue, Vector2* targetValue, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(Vector2* targetValue, CompressionScheme scheme, BitReader& reader);
};

} // namespace DAVA
