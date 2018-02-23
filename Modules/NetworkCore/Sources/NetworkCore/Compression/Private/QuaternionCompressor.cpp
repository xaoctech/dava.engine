#include "NetworkCore/Compression/Compression.h"
#include "NetworkCore/Compression/QuaternionCompressor.h"

#include <Base/BitReader.h>
#include <Base/BitWriter.h>
#include <Math/Quaternion.h>

namespace DAVA
{
CompressionScheme QuaternionCompressor::GetCompressionScheme(const ReflectedMeta* meta)
{
    return CompressionUtils::GetQuaternionCompressionSchemeFromMeta(meta);
}

float32 QuaternionCompressor::GetDeltaPrecision(const ReflectedMeta* meta)
{
    const CompressionScheme scheme = GetCompressionScheme(meta);
    return CompressionUtils::GetQuaternionDeltaPrecision(scheme);
}

float32 QuaternionCompressor::GetComparePrecision(const ReflectedMeta* meta)
{
    return CompressionUtils::GetComparePrecisionFromMeta(meta);
}

bool QuaternionCompressor::IsEqual(const Quaternion& value1, const Quaternion& value2, float32 comparePrecision)
{
    const float32 dx = std::abs(value2.x - value1.x);
    const float32 dy = std::abs(value2.y - value1.y);
    const float32 dz = std::abs(value2.z - value1.z);
    const float32 dw = std::abs(value2.w - value1.w);
    return dx <= comparePrecision && dy <= comparePrecision && dz <= comparePrecision && dw <= comparePrecision;
}

bool QuaternionCompressor::CompressDelta(const Quaternion& value1, const Quaternion& value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    const float32 deltaX = std::abs(value2.x - value1.x);
    const float32 deltaY = std::abs(value2.y - value1.y);
    const float32 deltaZ = std::abs(value2.z - value1.z);
    const float32 deltaW = std::abs(value2.w - value1.w);
    if (deltaX > deltaPrecision || deltaY > deltaPrecision || deltaZ > deltaPrecision || deltaW > deltaPrecision)
    {
        CompressFull(value2, scheme, deltaPrecision, writer);
        return true;
    }
    return false;
}

void QuaternionCompressor::CompressFull(const Quaternion& value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    const FracCompressionRecord* qRecord = scheme != 0 ? CompressionUtils::GetQuaternionRecord(scheme) : nullptr;
    CompressionUtils::CompressQuaternion(value, qRecord, writer);
}

void QuaternionCompressor::DecompressDelta(const Quaternion& /*baseValue*/, Quaternion* targetValue, CompressionScheme scheme, BitReader& reader)
{
    DecompressFull(targetValue, scheme, reader);
}

void QuaternionCompressor::DecompressFull(Quaternion* targetValue, CompressionScheme scheme, BitReader& reader)
{
    const FracCompressionRecord* qRecord = scheme != 0 ? CompressionUtils::GetQuaternionRecord(scheme) : nullptr;
    *targetValue = CompressionUtils::DecompressQuaternion(qRecord, reader);
}

} // namespace DAVA
