#pragma once

#include "NetworkCore/Compression/Compression.h"
#include <Base/BitReader.h>
#include <Base/BitWriter.h>
#include <Base/Bitset.h>
#include <Math/Math2D.h>
#include <Debug/DVAssert.h>

namespace DAVA
{
template <typename>
struct BitsetSizeExtractor;
template <size_t BITS>
struct BitsetSizeExtractor<Bitset<BITS>>
: public std::integral_constant<size_t, BITS>
{
};

template <typename TBitset>
class BitsetCompressor
{
    static constexpr size_t BITS = BitsetSizeExtractor<TBitset>();
    // Minimum number of bits for write the highest bit index.
    static constexpr size_t BITS_COUNT_PER_INDEX = StaticLog2<BITS>::value;
    using SetBits = Array<size_t, BITS>;

public:
    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(const TBitset& value1, const TBitset& value2, float32 /*comparePrecision*/);

    static bool CompressDelta(const TBitset& value1, const TBitset& value2, CompressionScheme scheme, float32 /*deltaPrecision*/, BitWriter& writer);
    static void CompressFull(const TBitset& value, CompressionScheme scheme, float32 /*deltaPrecision*/, BitWriter& writer);

    static void DecompressDelta(const TBitset& baseValue, TBitset* targetValue, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(TBitset* targetValue, CompressionScheme scheme, BitReader& reader);

private:
    static size_t GetRequiredBitsForPack(const TBitset& value, size_t& numberOfSetBits, SetBits& setBits);
};

template <typename TBitset>
CompressionScheme BitsetCompressor<TBitset>::GetCompressionScheme(const ReflectedMeta* meta)
{
    return 0;
}

template <typename TBitset>
float32 BitsetCompressor<TBitset>::GetDeltaPrecision(const ReflectedMeta* meta)
{
    return 0.f;
}

template <typename TBitset>
float32 BitsetCompressor<TBitset>::GetComparePrecision(const ReflectedMeta* meta)
{
    return 0.f;
}

template <typename TBitset>
bool BitsetCompressor<TBitset>::IsEqual(const TBitset& value1, const TBitset& value2, float32 /*comparePrecision*/)
{
    return value1 == value2;
}

template <typename TBitset>
bool BitsetCompressor<TBitset>::CompressDelta(const TBitset& value1, const TBitset& value2, CompressionScheme /*scheme*/, float32 /*deltaPrecision*/, BitWriter& writer)
{
    if (!IsEqual(value1, value2, 0.f))
    {
        TBitset deltaMask = value1 ^ value2;
        CompressFull(deltaMask, 0, 0.f, writer);
        return true;
    }
    return false;
}

template <typename TBitset>
void BitsetCompressor<TBitset>::CompressFull(const TBitset& value, CompressionScheme /*scheme*/, float32 /*deltaPrecision*/, BitWriter& writer)
{
    size_t numberOfSetBits;
    SetBits setBits;
    if (GetRequiredBitsForPack(value, numberOfSetBits, setBits) < BITS)
    {
        writer.WriteBits(true, 1);
        writer.WriteBits(numberOfSetBits, BITS_COUNT_PER_INDEX);
        for (size_t setBitIdx = 0; setBitIdx < numberOfSetBits; ++setBitIdx)
        {
            writer.WriteBits(setBits[setBitIdx], BITS_COUNT_PER_INDEX);
        }
    }
    else
    {
        writer.WriteBits(false, 1);
        for (size_t offset = 0; offset < BITS; ++offset)
        {
            writer.WriteBits(value[offset], 1);
        }
    }
}

template <typename TBitset>
void BitsetCompressor<TBitset>::DecompressDelta(const TBitset& baseValue, TBitset* targetValue, CompressionScheme /*scheme*/, BitReader& reader)
{
    DecompressFull(targetValue, 0, reader);
    (*targetValue) ^= baseValue;
}

template <typename TBitset>
void BitsetCompressor<TBitset>::DecompressFull(TBitset* targetValue, CompressionScheme /*scheme*/, BitReader& reader)
{
    targetValue->reset();
    const bool isPack = reader.ReadBits(1);
    if (isPack)
    {
        const size_t numberOfSetBits = reader.ReadBits(BITS_COUNT_PER_INDEX);
        for (size_t offset = 0; offset < numberOfSetBits; ++offset)
        {
            const size_t setBitIdx = reader.ReadBits(BITS_COUNT_PER_INDEX);
            targetValue->set(setBitIdx);
        }
    }
    else
    {
        for (size_t offset = 0; offset < BITS; ++offset)
        {
            if (reader.ReadBits(1))
            {
                targetValue->set(offset);
            }
        }
    }
}

template <typename TBitset>
size_t BitsetCompressor<TBitset>::GetRequiredBitsForPack(const TBitset& value, size_t& numberOfSetBits, SetBits& setBits)
{
    numberOfSetBits = 0;
    for (size_t offset = 0; offset < BITS; ++offset)
    {
        if (value[offset])
        {
            setBits[numberOfSetBits] = offset;
            ++numberOfSetBits;
        }
    }

    return ((numberOfSetBits + 1) * BITS_COUNT_PER_INDEX) + 1;
}
} // namespace DAVA
