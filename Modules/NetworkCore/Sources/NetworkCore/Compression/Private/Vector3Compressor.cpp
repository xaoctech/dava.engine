#include "NetworkCore/Compression/Vector3Compressor.h"

#include <Base/BitReader.h>
#include <Base/BitWriter.h>
#include <Math/Vector.h>

namespace DAVA
{
CompressionScheme Vector3Compressor::GetCompressionScheme(const ReflectedMeta* meta)
{
    return CompressionUtils::GetFloatCompressionSchemeFromMeta(meta);
}

float32 Vector3Compressor::GetDeltaPrecision(const ReflectedMeta* meta)
{
    const CompressionScheme scheme = GetCompressionScheme(meta);
    return CompressionUtils::GetFloatDeltaPrecision(scheme);
}

float32 Vector3Compressor::GetComparePrecision(const ReflectedMeta* meta)
{
    return CompressionUtils::GetComparePrecisionFromMeta(meta);
}

bool Vector3Compressor::IsEqual(const Vector3& value1, const Vector3& value2, float32 comparePrecision)
{
    const float32 dx = std::abs(value2.x - value1.x);
    const float32 dy = std::abs(value2.y - value1.y);
    const float32 dz = std::abs(value2.z - value1.z);
    return dx <= comparePrecision && dy <= comparePrecision && dz <= comparePrecision;
}

bool Vector3Compressor::CompressDelta(const Vector3& value1, const Vector3& value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    const float32 deltaX = std::abs(value2.x - value1.x);
    const float32 deltaY = std::abs(value2.y - value1.y);
    const float32 deltaZ = std::abs(value2.z - value1.z);
    if (deltaX > deltaPrecision || deltaY > deltaPrecision || deltaZ > deltaPrecision)
    {
        CompressFull(value2, scheme, deltaPrecision, writer);
        return true;
    }
    return false;
}

void Vector3Compressor::CompressFull(const Vector3& value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
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
    CompressionUtils::CompressFloat(value.z, deltaPrecision, intRecord, fracRecord, writer);
}

void Vector3Compressor::DecompressDelta(const Vector3& baseValue, Vector3* targetValue, CompressionScheme scheme, BitReader& reader)
{
    DecompressFull(targetValue, scheme, reader);
}

void Vector3Compressor::DecompressFull(Vector3* targetValue, CompressionScheme scheme, BitReader& reader)
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
    targetValue->z = CompressionUtils::DecompressFloat(intRecord, fracRecord, reader);
}
} // namespace DAVA
