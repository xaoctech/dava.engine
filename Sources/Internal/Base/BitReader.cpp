#include "Base/BitReader.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
BitReader::BitReader(const void* sourceBuffer, size_t sourceBufferSize)
    : buffer(static_cast<const uint32*>(sourceBuffer))
    , totalBitCount(static_cast<uint32>(sourceBufferSize / sizeof(uint32) + ((sourceBufferSize % sizeof(uint32)) != 0)) * 32)
{
    DVASSERT(buffer != nullptr);
    DVASSERT(sourceBufferSize > 0);

    accum = buffer[0];
}

uint32 BitReader::ReadBits(uint32 bits)
{
    DVASSERT(0 < bits && bits <= 32);
    //DVASSERT(bitsRead + bits <= totalBitCount);

    if (bitsRead + bits > totalBitCount)
    {
        overflowed = true;
        return 0;
    }

    uint32 result = 0;
    bitsRead += bits;
    if (bitIndex + bits <= 32)
    {
        result = accum >> bitIndex;
        if (bits != 32)
        {
            result &= (1 << bits) - 1;
        }

        bitIndex += bits;
        if (bitIndex == 32)
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

        const uint32 bitsB = bits - (32 - bitIndex);
        const uint32 mask = (1 << bitsB) - 1;
        result |= (accum & mask) << (32 - bitIndex);

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
    return wordIndex * 4 + bitIndex / 8 + ((bitIndex % 8) != 0);
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
