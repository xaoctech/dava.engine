#pragma once

#include "NetworkCore/Compression/Compression.h"
#include "NetworkCore/Compression/ArrayCompressorUtil.h"

#include <Debug/DVAssert.h>

namespace DAVA
{
class BitReader;
class BitWriter;

template <typename T, typename TCompressor>
class ArrayCompressor
{
public:
    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(const T* p1, const T* p2, size_t size, float32 comparePrecision);

    static bool CompressDelta(const T* p1, const T* p2, size_t size, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);
    static void CompressFull(const T* p, size_t size, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);

    static void DecompressDelta(const T* p1, T* p2, size_t size, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(T* p, size_t size, CompressionScheme scheme, BitReader& reader);
};

template <typename T, typename TCompressor>
CompressionScheme ArrayCompressor<T, TCompressor>::GetCompressionScheme(const ReflectedMeta* meta)
{
    return ArrayCompressorUtil<T, TCompressor>::GetCompressionScheme(meta);
}

template <typename T, typename TCompressor>
float32 ArrayCompressor<T, TCompressor>::GetDeltaPrecision(const ReflectedMeta* meta)
{
    return ArrayCompressorUtil<T, TCompressor>::GetDeltaPrecision(meta);
}

template <typename T, typename TCompressor>
float32 ArrayCompressor<T, TCompressor>::GetComparePrecision(const ReflectedMeta* meta)
{
    return ArrayCompressorUtil<T, TCompressor>::GetComparePrecision(meta);
}

template <typename T, typename TCompressor>
bool ArrayCompressor<T, TCompressor>::IsEqual(const T* p1, const T* p2, size_t size, float32 comparePrecision)
{
    return ArrayCompressorUtil<T, TCompressor>::IsEqual(p1, size, p2, size, comparePrecision);
}

template <typename T, typename TCompressor>
bool ArrayCompressor<T, TCompressor>::CompressDelta(const T* p1, const T* p2, size_t size, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    return ArrayCompressorUtil<T, TCompressor>::CompressDelta(p1, size, p2, size, scheme, deltaPrecision, writer);
}

template <typename T, typename TCompressor>
void ArrayCompressor<T, TCompressor>::CompressFull(const T* p, size_t size, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    ArrayCompressorUtil<T, TCompressor>::CompressFull(p, size, scheme, deltaPrecision, writer);
}

template <typename T, typename TCompressor>
void ArrayCompressor<T, TCompressor>::DecompressDelta(const T* p1, T* p2, size_t size, CompressionScheme scheme, BitReader& reader)
{
    const size_t sizeInStream = ArrayCompressorUtil<T, TCompressor>::DecompressSizeDelta(size, reader);
    DVASSERT(sizeInStream == size);
    ArrayCompressorUtil<T, TCompressor>::DecompressDelta(p1, size, p2, size, scheme, reader);
}

template <typename T, typename TCompressor>
void ArrayCompressor<T, TCompressor>::DecompressFull(T* p, size_t size, CompressionScheme scheme, BitReader& reader)
{
    const size_t sizeInStream = ArrayCompressorUtil<T, TCompressor>::DecompressSizeFull(reader);
    DVASSERT(sizeInStream == size);
    ArrayCompressorUtil<T, TCompressor>::DecompressFull(p, size, scheme, reader);
}

} // namespace DAVA
