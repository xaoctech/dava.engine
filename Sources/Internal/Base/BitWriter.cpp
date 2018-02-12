#include "Base/BitWriter.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
BitWriter::BitWriter(void* destinationBuffer, size_t destinationBufferSize)
    : buffer(static_cast<uint32*>(destinationBuffer))
    , totalBitCount(static_cast<uint32>(destinationBufferSize / sizeof(uint32) * 32))
{
    DVASSERT(buffer != nullptr);
    DVASSERT(destinationBufferSize >= sizeof(uint32));
}

void BitWriter::WriteBits(uint32 value, uint32 bits)
{
    DVASSERT(0 < bits && bits <= 32);
    //DVASSERT(bitsWritten + bits <= totalBitCount);

    if (bitsWritten + bits > totalBitCount)
    {
        overflowed = true;
        return;
    }

    if (bits < 32)
    {
        value &= (1 << bits) - 1;
    }

    bitsWritten += bits;
    if (bitIndex + bits <= 32)
    {
        accum |= value << bitIndex;
        bitIndex += bits;

        if (bitIndex == 32)
        {
            buffer[wordIndex++] = accum;
            bitIndex = 0;
            accum = 0;
        }
    }
    else
    {
        accum |= value << bitIndex;
        buffer[wordIndex++] = accum;

        accum = value >> (32 - bitIndex);
        bitIndex = bits - (32 - bitIndex);
    }
}

void BitWriter::PatchBits(uint32 offset, uint32 value, uint32 bits)
{
    DVASSERT(0 < bits && bits <= 32);
    DVASSERT(offset + bits <= bitsWritten);

    uint32 w = offset / 32;
    uint32 i = offset % 32;

    const uint64 value64 = (static_cast<uint64>(value) & ((1ull << bits) - 1)) << i;
    const uint64 mask64 = ~(((1ull << bits) - 1) << i);
    const uint32 mask1 = static_cast<uint32>(mask64);
    const uint32 mask2 = static_cast<uint32>(mask64 >> 32);

    uint32 v = w == wordIndex ? accum : buffer[w];
    v &= mask1;
    v |= static_cast<uint32>(value64);
    w == wordIndex ? accum = v : buffer[w] = v;

    if (~mask2 != 0)
    {
        w += 1;
        uint32 v = w == wordIndex ? accum : buffer[w];
        v &= mask2;
        v |= static_cast<uint32>(value64 >> 32);
        w == wordIndex ? accum = v : buffer[w] = v;
    }
}

void BitWriter::WriteAlignmentBits()
{
    const uint32 remainingBits = bitsWritten % 8;
    if (remainingBits != 0)
    {
        WriteBits(0, 8 - remainingBits);
    }
}

void BitWriter::Rewind(uint32 bits)
{
    DVASSERT(0 < bits && bits <= bitsWritten);

    if (bits <= bitIndex)
    {
        bitsWritten -= bits;
        bitIndex -= bits;
        accum &= (1 << bitIndex) - 1;
    }
    else
    {
        bitsWritten -= bits;
        bitIndex = bitsWritten % 32;
        wordIndex = bitsWritten / 32;
        accum = buffer[wordIndex];
        accum &= (1 << bitIndex) - 1;
    }
}

void BitWriter::Flush()
{
    if (bitIndex != 0 && bitsWritten < totalBitCount)
    {
        buffer[wordIndex] = accum;
    }
}

uint32 BitWriter::GetBytesWritten() const
{
    return wordIndex * 4 + bitIndex / 8 + ((bitIndex % 8) != 0);
}

uint32 BitWriter::GetBitsWritten() const
{
    return bitsWritten;
}

bool BitWriter::IsOverflowed() const
{
    return overflowed;
}

} // namespace DAVA
