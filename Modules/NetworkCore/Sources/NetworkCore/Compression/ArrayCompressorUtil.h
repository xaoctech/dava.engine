#pragma once

#include "NetworkCore/Compression/Compression.h"
#include <Base/BitReader.h>
#include <Base/BitWriter.h>
#include <Debug/DVAssert.h>
#include <Math/MathHelpers.h>

#include <algorithm>

#include <bitset>

namespace DAVA
{
class BitReader;
class BitWriter;
class ReflectedMeta;

struct ArrayAnalyzeResult
{
    uint32 size;
    uint32 newSize;
    bool sizeChanged;
    uint32 nchanged; // total changed elements
    uint32 indexFirst; // index of first changed element
    uint32 indexLast; // index of last changed element
    uint32 maxIndexDelta; // maximum distance between changed elements
    std::bitset<maxArraySize> changedElements;

    uint32 bitsPerNewSize; // bits required to store array size
    uint32 bitsPerNChanged; // bits required to store number of changed elements
    uint32 bitsPerIndexFirst; // bits required to store indexFirst
    uint32 bitsPerIndexDelta; // bits required to store maxIndexDelta
};

enum CompressStrategy
{
    STRATEGY_BY_INDEX = 0,
    STRATEGY_BY_FLAG,
};

template <typename T, typename TCompressor>
struct ArrayCompressorUtil
{
    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(const T* p1, size_t size1, const T* p2, size_t size2, float32 comparePrecision);

    static bool CompressDelta(const T* p1, size_t size1, const T* p2, size_t size2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);
    static void CompressFull(const T* p, size_t size, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);

    static size_t DecompressSizeDelta(size_t size1, BitReader& bitReader);
    static size_t DecompressSizeFull(BitReader& bitReader);

    static void DecompressDelta(const T* p1, size_t size1, T* p2, size_t size2, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(T* p, size_t size, CompressionScheme scheme, BitReader& reader);

    static ArrayAnalyzeResult Analyze(const T* p1, size_t size1, const T* p2, size_t size2, float32 deltaPrecision);
    static CompressStrategy SelectCompressStrategy(ArrayAnalyzeResult& r);
};

template <typename T, typename TCompressor>
CompressionScheme ArrayCompressorUtil<T, TCompressor>::GetCompressionScheme(const ReflectedMeta* meta)
{
    return TCompressor::GetCompressionScheme(meta);
}

template <typename T, typename TCompressor>
float32 ArrayCompressorUtil<T, TCompressor>::GetDeltaPrecision(const ReflectedMeta* meta)
{
    return TCompressor::GetDeltaPrecision(meta);
}

template <typename T, typename TCompressor>
float32 ArrayCompressorUtil<T, TCompressor>::GetComparePrecision(const ReflectedMeta* meta)
{
    return TCompressor::GetComparePrecision(meta);
}

template <typename T, typename TCompressor>
bool ArrayCompressorUtil<T, TCompressor>::IsEqual(const T* p1, size_t size1, const T* p2, size_t size2, float32 comparePrecision)
{
    DVASSERT(p1 != nullptr && p2 != nullptr);
    DVASSERT(0 <= size1 && size1 <= maxArraySize);
    DVASSERT(0 <= size2 && size2 <= maxArraySize);

    if (size1 == size2)
    {
        for (size_t i = 0; i < size1; ++i)
        {
            if (!TCompressor::IsEqual(p1[i], p2[i], comparePrecision))
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

template <typename T, typename TCompressor>
bool ArrayCompressorUtil<T, TCompressor>::CompressDelta(const T* p1, size_t size1, const T* p2, size_t size2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    DVASSERT(p1 != nullptr && p2 != nullptr);
    DVASSERT(0 <= size1 && size1 <= maxArraySize);
    DVASSERT(0 <= size2 && size2 <= maxArraySize);

    ArrayAnalyzeResult r = Analyze(p1, size1, p2, size2, deltaPrecision);
    if (r.nchanged > 0 || r.sizeChanged)
    {
        const CompressStrategy strategy = SelectCompressStrategy(r);

        writer.WriteBits(r.sizeChanged, 1);
        if (r.sizeChanged)
        {
            writer.WriteBits(r.bitsPerNewSize - 1, 3);
            writer.WriteBits(r.newSize, r.bitsPerNewSize);
        }

        writer.WriteBits(static_cast<uint32>(strategy), 1);
        if (strategy == STRATEGY_BY_INDEX)
        {
            writer.WriteBits(r.bitsPerNChanged - 1, 3);
            writer.WriteBits(r.nchanged, r.bitsPerNChanged);

            if (r.nchanged > 0)
            {
                writer.WriteBits(r.bitsPerIndexFirst, 4);
                writer.WriteBits(r.indexFirst, r.bitsPerIndexFirst);
                TCompressor::CompressDelta(p1[r.indexFirst], p2[r.indexFirst], scheme, deltaPrecision, writer);

                if (r.nchanged > 1)
                {
                    uint32 baseIndex = r.indexFirst;
                    writer.WriteBits(r.bitsPerIndexDelta - 1, 3);
                    for (uint32 i = r.indexFirst + 1; i <= r.indexLast; ++i)
                    {
                        if (r.changedElements.test(i))
                        {
                            writer.WriteBits(i - baseIndex, r.bitsPerIndexDelta);
                            TCompressor::CompressDelta(p1[i], p2[i], scheme, deltaPrecision, writer);
                            baseIndex = i;
                        }
                    }
                }
            }
        }
        else
        {
            writer.WriteBits(static_cast<uint32>(r.nchanged > 0), 1);
            if (r.nchanged > 0)
            {
                for (uint32 i = 0; i < r.size; ++i)
                {
                    const bool isChanged = r.changedElements[i];
                    writer.WriteBits(isChanged, 1);
                    if (isChanged)
                    {
                        TCompressor::CompressDelta(p1[i], p2[i], scheme, deltaPrecision, writer);
                    }
                }
            }
        }

        for (size_t i = size1; i < size2; ++i)
        {
            TCompressor::CompressFull(p2[i], scheme, deltaPrecision, writer);
        }
        return true;
    }
    return false;
}

template <typename T, typename TCompressor>
void ArrayCompressorUtil<T, TCompressor>::CompressFull(const T* p, size_t size, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    DVASSERT(p != nullptr);
    DVASSERT(0 <= size && size <= maxArraySize);

    writer.WriteBits(static_cast<uint32>(size), 8);
    for (size_t i = 0; i < size; ++i)
    {
        TCompressor::CompressFull(p[i], scheme, deltaPrecision, writer);
    }
}

template <typename T, typename TCompressor>
size_t ArrayCompressorUtil<T, TCompressor>::DecompressSizeDelta(size_t size1, BitReader& reader)
{
    size_t size = size1;
    const bool sizeChanged = reader.ReadBits(1) != 0;
    if (sizeChanged)
    {
        const uint32 bitsPerNewSize = reader.ReadBits(3) + 1;
        size = reader.ReadBits(bitsPerNewSize);
        DVASSERT(0 <= size && size <= maxArraySize);
    }
    return size;
}

template <typename T, typename TCompressor>
size_t ArrayCompressorUtil<T, TCompressor>::DecompressSizeFull(BitReader& reader)
{
    size_t size = reader.ReadBits(8);
    DVASSERT(0 <= size && size <= maxArraySize);
    return size;
}

template <typename T, typename TCompressor>
void ArrayCompressorUtil<T, TCompressor>::DecompressDelta(const T* p1, size_t size1, T* p2, size_t size2, CompressionScheme scheme, BitReader& reader)
{
    DVASSERT(p1 != nullptr && p2 != nullptr);
    DVASSERT(0 <= size1 && size1 <= maxArraySize);
    DVASSERT(0 <= size2 && size2 <= maxArraySize);

    const CompressStrategy strategy = reader.ReadBits(1) == 0 ? STRATEGY_BY_INDEX : STRATEGY_BY_FLAG;
    if (strategy == STRATEGY_BY_INDEX)
    {
        const uint32 bitsPerNChanged = reader.ReadBits(3) + 1;
        const uint32 nchanged = reader.ReadBits(bitsPerNChanged);
        if (nchanged > 0)
        {
            const uint32 bitsPerIndexFirst = reader.ReadBits(4);
            uint32 indexFirst = reader.ReadBits(bitsPerIndexFirst);
            TCompressor::DecompressDelta(p1[indexFirst], &p2[indexFirst], scheme, reader);

            if (nchanged > 1)
            {
                uint32 baseIndex = indexFirst;
                const uint32 bitsPerIndexDelta = reader.ReadBits(3) + 1;
                for (uint32 i = 0; i < nchanged - 1; ++i)
                {
                    const uint32 indexDelta = reader.ReadBits(bitsPerIndexDelta);
                    const uint32 index = indexDelta + baseIndex;
                    TCompressor::DecompressDelta(p1[index], &p2[index], scheme, reader);
                    baseIndex = index;
                }
            }
        }
    }
    else
    {
        const bool smthChanged = reader.ReadBits(1) != 0;
        if (smthChanged)
        {
            const uint32 size = static_cast<uint32>(std::min(size1, size2));
            for (uint32 i = 0; i < size; ++i)
            {
                const bool isChanged = reader.ReadBits(1) != 0;
                if (isChanged)
                {
                    TCompressor::DecompressDelta(p1[i], &p2[i], scheme, reader);
                }
            }
        }
    }

    for (size_t i = size1; i < size2; ++i)
    {
        TCompressor::DecompressFull(&p2[i], scheme, reader);
    }
}

template <typename T, typename TCompressor>
void ArrayCompressorUtil<T, TCompressor>::DecompressFull(T* p, size_t size, CompressionScheme scheme, BitReader& reader)
{
    DVASSERT(p != nullptr);
    DVASSERT(0 <= size && size <= maxArraySize);

    for (size_t i = 0; i < size; ++i)
    {
        TCompressor::DecompressFull(&p[i], scheme, reader);
    }
}

template <typename T, typename TCompressor>
ArrayAnalyzeResult ArrayCompressorUtil<T, TCompressor>::Analyze(const T* p1, size_t size1, const T* p2, size_t size2, float32 deltaPrecision)
{
    ArrayAnalyzeResult result{};
    result.size = static_cast<uint32>(std::min(size1, size2));
    result.newSize = static_cast<uint32>(size2);
    result.sizeChanged = size1 != size2;

    // Find first changed element
    for (; result.indexFirst < result.size; ++result.indexFirst)
    {
        if (!TCompressor::IsEqual(p1[result.indexFirst], p2[result.indexFirst], deltaPrecision))
        {
            result.changedElements.set(result.indexFirst);
            result.nchanged += 1;
            break;
        }
    }

    // Find the rest changed elements, if any
    result.indexLast = result.indexFirst;
    for (uint32 i = result.indexFirst + 1; i < result.size; ++i)
    {
        if (!TCompressor::IsEqual(p1[i], p2[i], deltaPrecision))
        {
            result.changedElements.set(i);
            result.maxIndexDelta = std::max(result.maxIndexDelta, i - result.indexLast);
            result.nchanged += 1;
            result.indexLast = i;
        }
    }
    return result;
}

template <typename T, typename TCompressor>
CompressStrategy ArrayCompressorUtil<T, TCompressor>::SelectCompressStrategy(ArrayAnalyzeResult& r)
{
    r.bitsPerNewSize = r.newSize > 0 ? GetBitsRequired(r.newSize) : 1;
    r.bitsPerNChanged = r.nchanged > 0 ? GetBitsRequired(r.nchanged) : 1;

    r.bitsPerIndexFirst = r.indexFirst > 0 ? GetBitsRequired(r.indexFirst) : 1;
    r.bitsPerIndexDelta = GetBitsRequired(r.maxIndexDelta);

    // clang-format off

    /*
        Strategy by-index:
        1 bit   required    size_changed_flag   flag indicating whether array size is changed

        if size is changed:
        3 bits  required    number of bits used to store new array size (0 - 1 bits, 1 - 2 bits, ..., 7 - 8 bits)
        N bits  required    new array size

        3 bits  required    number of bits to store changed elements count
        N bits  required    count of changed elements

        if nchanged > 0:
        4 bits  required    number of bits to store index of first changed element
        N bits  required    index of first changed element
            value delta of first element
        if nchanged > 1:
        3 bits  required    number of bits to store difference between indices of changed elements (index delta)
        N bits  required    index delta
            value delta of element[delta + prev]

        if size is changed and new size is greater then previous:
            full value of element

        Strategy by-flag:
        1 bit   required    size_changed_flag   flag indicating whether array size is changed

        if size is changed:
        3 bits  required    number of bits used to store new array size (0 - 1 bits, 1 - 2 bits, ..., 7 - 8 bits)
        N bits  required    new array size

        1 bit   required    flag indicating that at least one element has changed

        if nchanged > 0:
        1 bit   required    flag indicating whether value is changed
            value delta of element

        if size is changed and new size is greater then previous:
            full value of element
    */


    const uint32 overheadByIndex =
        1 +
        (r.sizeChanged ? 3 + r.bitsPerNewSize : 0) +
        3 + r.bitsPerNChanged +
        (r.nchanged > 0 ? 3 + r.bitsPerIndexFirst : 0) + 
        (r.nchanged > 1 ? 3 + (r.nchanged - 1) * r.bitsPerIndexDelta : 0) +
        0;

    const uint32 overheadByFlag =
        1 +
        (r.sizeChanged ? 3 + r.bitsPerNewSize : 0) +
        (r.nchanged > 0 ? 1 + r.size : 0);

    // clang-format on
    return overheadByIndex <= overheadByFlag ? STRATEGY_BY_INDEX : STRATEGY_BY_FLAG;
}

} // namespace DAVA
