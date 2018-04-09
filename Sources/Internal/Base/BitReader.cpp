#include "Base/BitReader.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
BitReader::BitReader(const void* sourceBuffer, size_t sourceBufferSize)
    : buffer(static_cast<const AccumulatorType*>(sourceBuffer))
    , totalBitCount(static_cast<uint32>(sourceBufferSize / AccumulatorTypeByteSize + ((sourceBufferSize % AccumulatorTypeByteSize) != 0)) * AccumulatorTypeBitSize)
{
    DVASSERT(buffer != nullptr);
    DVASSERT(sourceBufferSize > 0);

    accum = buffer[0];
}

uint32 BitReader::ReadBits(uint32 bits)
{
    DVASSERT(0 < bits && bits <= AccumulatorTypeBitSize);
    //DVASSERT(bitsRead + bits <= totalBitCount);

    if (bitsRead + bits > totalBitCount)
    {
        overflowed = true;
        return 0;
    }

    uint32 result = 0;
    bitsRead += bits;
    if (bitIndex + bits <= AccumulatorTypeBitSize)
    {
        result = accum >> bitIndex;
        if (bits != AccumulatorTypeBitSize)
        {
            result &= (1 << bits) - 1;
        }

        bitIndex += bits;
        if (bitIndex == AccumulatorTypeBitSize)
        {
            if (bitsRead < totalBitCount)
            {
                bitIndex = 0;
                accum = buffer[++wordIndex];
            }
        }
    }
    else
    {
        result = accum >> bitIndex;

        accum = buffer[++wordIndex];

        const uint32 bitsB = bits - (AccumulatorTypeBitSize - bitIndex);
        const uint32 mask = (1 << bitsB) - 1;
        result |= (accum & mask) << (AccumulatorTypeBitSize - bitIndex);

        bitIndex = bitsB;
    }
    return result;
}

void BitReader::ReadAlignmentBits()
{
    const uint32 remainingBits = static_cast<uint32>(bitsRead % 8);
    if (remainingBits != 0)
    {
        ReadBits(8 - remainingBits);
    }
}

uint32 BitReader::GetBytesRead() const
{
    return wordIndex * AccumulatorTypeByteSize + bitIndex / 8 + ((bitIndex % 8) != 0);
}

uint32 BitReader::GetBitsRead() const
{
    return bitsRead;
}

bool BitReader::IsOverflowed() const
{
    return overflowed;
}

} // namespace DAVA
