#include "NetworkCore/Compression/CompMaskCompressor.h"
#include "NetworkCore/Compression/BitsetCompressor.h"

#include <Base/BitReader.h>
#include <Base/BitWriter.h>

namespace DAVA
{
CompressionScheme CompMaskCompressor::GetCompressionScheme(const ReflectedMeta* meta)
{
    return BitsetCompressor<MaskBitSet>::GetCompressionScheme(meta);
}

float32 CompMaskCompressor::GetDeltaPrecision(const ReflectedMeta* meta)
{
    return BitsetCompressor<MaskBitSet>::GetDeltaPrecision(meta);
}

float32 CompMaskCompressor::GetComparePrecision(const ReflectedMeta* meta)
{
    return BitsetCompressor<MaskBitSet>::GetComparePrecision(meta);
}

bool CompMaskCompressor::IsEqual(ComponentMask value1, ComponentMask value2, float32 comparePrecision)
{
    return BitsetCompressor<MaskBitSet>::IsEqual(value1.bits, value2.bits, comparePrecision);
}

bool CompMaskCompressor::CompressDelta(ComponentMask value1, ComponentMask value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    return BitsetCompressor<MaskBitSet>::CompressDelta(value1.bits, value2.bits, scheme, deltaPrecision, writer);
}

void CompMaskCompressor::CompressFull(ComponentMask value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    BitsetCompressor<MaskBitSet>::CompressFull(value.bits, scheme, deltaPrecision, writer);
}

void CompMaskCompressor::DecompressDelta(ComponentMask baseValue, ComponentMask* targetValue, CompressionScheme scheme, BitReader& reader)
{
    BitsetCompressor<MaskBitSet>::DecompressDelta(baseValue.bits, &targetValue->bits, scheme, reader);
}

void CompMaskCompressor::DecompressFull(ComponentMask* targetValue, CompressionScheme scheme, BitReader& reader)
{
    BitsetCompressor<MaskBitSet>::DecompressFull(&targetValue->bits, scheme, reader);
}

} // namespace DAVA
