#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class BitWriter
{
public:
    BitWriter(void* destinationBuffer, size_t destinationBufferSize);

    void WriteBits(uint32 value, uint32 bits);
    void PatchBits(uint32 offset, uint32 value, uint32 bits);
    void WriteAlignmentBits();
    void Rewind(uint32 bits);
    void Flush();

    uint32 GetBytesWritten() const;
    uint32 GetBitsWritten() const;
    bool IsOverflowed() const;

private:
    uint32* buffer = nullptr;
    uint32 totalBitCount = 0;

    uint32 bitsWritten = 0;
    uint32 wordIndex = 0;
    uint32 bitIndex = 0;
    uint32 accum = 0;
    bool overflowed = false;
};

} // namespace DAVA
