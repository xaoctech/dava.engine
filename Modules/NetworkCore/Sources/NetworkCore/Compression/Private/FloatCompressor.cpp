#include "NetworkCore/Compression/FloatCompressor.h"

#include <Base/BitReader.h>
#include <Base/BitWriter.h>

namespace DAVA
{
CompressionScheme FloatCompressor::GetCompressionScheme(const ReflectedMeta* meta)
{
    return CompressionUtils::GetFloatCompressionSchemeFromMeta(meta);
}

float32 FloatCompressor::GetDeltaPrecision(const ReflectedMeta* meta)
{
    const CompressionScheme scheme = GetCompressionScheme(meta);
    return CompressionUtils::GetFloatDeltaPrecision(scheme);
}

float32 FloatCompressor::GetComparePrecision(const ReflectedMeta* meta)
{
    return CompressionUtils::GetComparePrecisionFromMeta(meta);
}

bool FloatCompressor::IsEqual(float32 value1, float32 value2, float32 comparePrecision)
{
    const float32 delta = std::abs(value2 - value1);
    return delta <= comparePrecision;
}

bool FloatCompressor::CompressDelta(float32 value1, float32 value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    const float32 delta = std::abs(value2 - value1);
    if (delta > deltaPrecision)
    {
        CompressFull(value2, scheme, deltaPrecision, writer);
        return true;
    }
    return false;
}

void FloatCompressor::CompressFull(float32 value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    const IntCompressionRecord* intRecord = nullptr;
    const FracCompressionRecord* fracRecord = nullptr;
    if (scheme != 0)
    {
        intRecord = CompressionUtils::GetFullRangeRecord(scheme);
        fracRecord = CompressionUtils::GetFracRecord(scheme);
    }
    CompressionUtils::CompressFloat(value, deltaPrecision, intRecord, fracRecord, writer);
}

void FloatCompressor::DecompressDelta(float32 baseValue, float32* targetValue, CompressionScheme scheme, BitReader& reader)
{
    DecompressFull(targetValue, scheme, reader);
}

void FloatCompressor::DecompressFull(float32* targetValue, CompressionScheme scheme, BitReader& reader)
{
    const IntCompressionRecord* intRecord = nullptr;
    const FracCompressionRecord* fracRecord = nullptr;
    if (scheme != 0)
    {
        intRecord = CompressionUtils::GetFullRangeRecord(scheme);
        fracRecord = CompressionUtils::GetFracRecord(scheme);
    }
    float32 value = CompressionUtils::DecompressFloat(intRecord, fracRecord, reader);
    *targetValue = value;
}

} // namespace DAVA
