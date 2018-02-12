#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class BitReader
{
public:
    BitReader(const void* sourceBuffer, size_t sourceBufferSize);

    uint32 ReadBits(uint32 bits);
    void ReadAlignmentBits();

    uint32 GetBytesRead() const;
    uint32 GetBitsRead() const;
    bool IsOverflowed() const;

private:
    const uint32* buffer = nullptr;
    uint32 totalBitCount = 0;

    uint32 bitsRead = 0;
    uint32 wordIndex = 0;
    uint32 bitIndex = 0;
    uint32 accum = 0;
    bool overflowed = false;
};

} // namespace DAVA
