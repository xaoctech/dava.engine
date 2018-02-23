#pragma once

#include <Base/BaseTypes.h>
#include <Base/BitReader.h>
#include <Base/BitWriter.h>
#include <Base/FixedVector.h>
#include <Debug/DVAssert.h>

#include "NetworkCore/Compression/ArrayCompressorUtil.h"

namespace DAVA
{
class BitReader;
class BitWriter;
class ReflectedMeta;

template <typename T, typename TCompressor>
class FixedVectorCompressor
{
public:
    using ACompressor = ArrayCompressorUtil<T, TCompressor>;

    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(const FixedVector<T>& vector1, const FixedVector<T>& vector2, float32 comparePrecision);

    static bool CompressDelta(const FixedVector<T>& vector1, const FixedVector<T>& vector2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);
    static void CompressFull(const FixedVector<T>& vector, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);

    static void DecompressDelta(const FixedVector<T>& baseVector, FixedVector<T>* targetVector, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(FixedVector<T>* targetVector, CompressionScheme scheme, BitReader& reader);
};

template <typename T, typename TCompressor>
CompressionScheme FixedVectorCompressor<T, TCompressor>::GetCompressionScheme(const ReflectedMeta* meta)
{
    return ACompressor::GetCompressionScheme(meta);
}

template <typename T, typename TCompressor>
float32 FixedVectorCompressor<T, TCompressor>::GetDeltaPrecision(const ReflectedMeta* meta)
{
    return ACompressor::GetDeltaPrecision(meta);
}

template <typename T, typename TCompressor>
float32 FixedVectorCompressor<T, TCompressor>::GetComparePrecision(const ReflectedMeta* meta)
{
    return ACompressor::GetComparePrecision(meta);
}

template <typename T, typename TCompressor>
bool FixedVectorCompressor<T, TCompressor>::IsEqual(const FixedVector<T>& vector1, const FixedVector<T>& vector2, float32 comparePrecision)
{
    DVASSERT(vector1.max_size() == vector2.max_size());
    return ACompressor::IsEqual(vector1.data(), vector1.size(), vector2.data(), vector2.size(), comparePrecision);
}

template <typename T, typename TCompressor>
bool FixedVectorCompressor<T, TCompressor>::CompressDelta(const FixedVector<T>& vector1, const FixedVector<T>& vector2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    DVASSERT(vector1.max_size() == vector2.max_size());

    const T* p1 = vector1.data();
    const size_t size1 = vector1.size();
    const T* p2 = vector2.data();
    const size_t size2 = vector2.size();
    return ACompressor::CompressDelta(p1, size1, p2, size2, scheme, deltaPrecision, writer);
}

template <typename T, typename TCompressor>
void FixedVectorCompressor<T, TCompressor>::CompressFull(const FixedVector<T>& vector, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    const T* seq = vector.data();
    const size_t size = vector.size();
    ACompressor::CompressFull(seq, size, scheme, deltaPrecision, writer);
}

template <typename T, typename TCompressor>
void FixedVectorCompressor<T, TCompressor>::DecompressDelta(const FixedVector<T>& baseVector, FixedVector<T>* targetVector, CompressionScheme scheme, BitReader& reader)
{
    DVASSERT(baseVector.max_size() == targetVector->max_size());

    const size_t baseSize = baseVector.size();
    const T* baseSeq = baseVector.data();

    const size_t targetSize = ACompressor::DecompressSizeDelta(baseSize, reader);
    T* targetSeq = targetVector->data();
    targetVector->resize(targetSize);

    ACompressor::DecompressDelta(baseSeq, baseSize, targetSeq, targetSize, scheme, reader);
}

template <typename T, typename TCompressor>
void FixedVectorCompressor<T, TCompressor>::DecompressFull(FixedVector<T>* targetVector, CompressionScheme scheme, BitReader& reader)
{
    T* targetSeq = targetVector->data();
    const size_t targetSize = ACompressor::DecompressSizeFull(reader);
    targetVector->resize(targetSize);

    ACompressor::DecompressFull(targetSeq, targetSize, scheme, reader);
}

} // namespace DAVA
