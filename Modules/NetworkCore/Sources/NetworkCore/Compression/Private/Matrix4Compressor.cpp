#include "NetworkCore/Compression/ArrayCompressor.h"
#include "NetworkCore/Compression/FloatCompressor.h"
#include "NetworkCore/Compression/Matrix4Compressor.h"

#include <Base/BitReader.h>
#include <Base/BitWriter.h>
#include <Math/Matrix4.h>

namespace DAVA
{
CompressionScheme Matrix4Compressor::GetCompressionScheme(const ReflectedMeta* meta)
{
    return ArrayCompressor<float32, FloatCompressor>::GetCompressionScheme(meta);
}

float32 Matrix4Compressor::GetDeltaPrecision(const ReflectedMeta* meta)
{
    return ArrayCompressor<float32, FloatCompressor>::GetDeltaPrecision(meta);
}

float32 Matrix4Compressor::GetComparePrecision(const ReflectedMeta* meta)
{
    return ArrayCompressor<float32, FloatCompressor>::GetComparePrecision(meta);
}

bool Matrix4Compressor::IsEqual(const Matrix4& value1, const Matrix4& value2, float32 comparePrecision)
{
    return ArrayCompressor<float32, FloatCompressor>::IsEqual(value1.data, value2.data, 16, comparePrecision);
}

bool Matrix4Compressor::CompressDelta(const Matrix4& value1, const Matrix4& value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    return ArrayCompressor<float32, FloatCompressor>::CompressDelta(value1.data, value2.data, 16, scheme, deltaPrecision, writer);
}

void Matrix4Compressor::CompressFull(const Matrix4& value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    ArrayCompressor<float32, FloatCompressor>::CompressFull(value.data, 16, scheme, deltaPrecision, writer);
}

void Matrix4Compressor::DecompressDelta(const Matrix4& baseValue, Matrix4* targetValue, CompressionScheme scheme, BitReader& reader)
{
    ArrayCompressor<float32, FloatCompressor>::DecompressDelta(baseValue.data, targetValue->data, 16, scheme, reader);
}

void Matrix4Compressor::DecompressFull(Matrix4* targetValue, CompressionScheme scheme, BitReader& reader)
{
    ArrayCompressor<float32, FloatCompressor>::DecompressFull(targetValue->data, 16, scheme, reader);
}
} // namespace DAVA
