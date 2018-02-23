#pragma once

#include "NetworkCore/Compression/Compression.h"

namespace DAVA
{
class BitReader;
class BitWriter;
class ReflectedMeta;
struct Matrix4;

class Matrix4Compressor
{
public:
    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(const Matrix4& value1, const Matrix4& value2, float32 comparePrecision);

    static bool CompressDelta(const Matrix4& value1, const Matrix4& value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);
    static void CompressFull(const Matrix4& value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);

    static void DecompressDelta(const Matrix4& baseValue, Matrix4* targetValue, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(Matrix4* targetValue, CompressionScheme scheme, BitReader& reader);
};

} // namespace DAVA
