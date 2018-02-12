#include "NetworkCore/Compression/Vector2Compressor.h"

#include <Base/BitReader.h>
#include <Base/BitWriter.h>
#include <Math/Vector.h>

namespace DAVA
{
CompressionScheme Vector2Compressor::GetCompressionScheme(const ReflectedMeta* meta)
{
    return CompressionUtils::GetFloatCompressionSchemeFromMeta(meta);
}

float32 Vector2Compressor::GetDeltaPrecision(const ReflectedMeta* meta)
{
    const CompressionScheme scheme = GetCompressionScheme(meta);
    return CompressionUtils::GetFloatDeltaPrecision(scheme);
}

float32 Vector2Compressor::GetComparePrecision(const ReflectedMeta* meta)
{
    return CompressionUtils::GetComparePrecisionFromMeta(meta);
}

bool Vector2Compressor::IsEqual(const Vector2& value1, const Vector2& value2, float32 comparePrecision)
{
    const float32 dx = std::abs(value2.x - value1.x);
    const float32 dy = std::abs(value2.y - value1.y);
    return dx <= comparePrecision && dy <= comparePrecision;
}

bool Vector2Compressor::CompressDelta(const Vector2& value1, const Vector2& value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    const float32 deltaX = std::abs(value2.x - value1.x);
    const float32 deltaY = std::abs(value2.y - value1.y);
    if (deltaX > deltaPrecision || deltaY > deltaPrecision)
    {
        CompressFull(value2, scheme, deltaPrecision, writer);
        return true;
    }
    return false;
}

void Vector2Compressor::CompressFull(const Vector2& value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    const IntCompressionRecord* intRecord = nullptr;
    const FracCompressionRecord* fracRecord = nullptr;
    if (scheme != 0)
    {
        intRecord = CompressionUtils::GetFullRangeRecord(scheme);
        fracRecord = CompressionUtils::GetFracRecord(scheme);
    }
    CompressionUtils::CompressFloat(value.x, deltaPrecision, intRecord, fracRecord, writer);
    CompressionUtils::CompressFloat(value.y, deltaPrecision, intRecord, fracRecord, writer);
}

void Vector2Compressor::DecompressDelta(const Vector2& baseValue, Vector2* targetValue, CompressionScheme scheme, BitReader& reader)
{
    DecompressFull(targetValue, scheme, reader);
}

void Vector2Compressor::DecompressFull(Vector2* targetValue, CompressionScheme scheme, BitReader& reader)
{
    const IntCompressionRecord* intRecord = nullptr;
    const FracCompressionRecord* fracRecord = nullptr;
    if (scheme != 0)
    {
        intRecord = CompressionUtils::GetFullRangeRecord(scheme);
        fracRecord = CompressionUtils::GetFracRecord(scheme);
    }

    targetValue->x = CompressionUtils::DecompressFloat(intRecord, fracRecord, reader);
    targetValue->y = CompressionUtils::DecompressFloat(intRecord, fracRecord, reader);
}
} // namespace DAVA
