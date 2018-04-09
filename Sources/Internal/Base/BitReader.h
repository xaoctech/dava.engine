#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class BitReader
{
public:
    using AccumulatorType = uint32;
    static_assert(sizeof(AccumulatorType) == sizeof(uint32) && std::is_unsigned<AccumulatorType>::value, "Type should be uint32");
    const static uint8 AccumulatorTypeByteSize = sizeof(AccumulatorType);
    const static uint8 AccumulatorTypeBitSize = sizeof(AccumulatorType) * 8;

    BitReader(const void* sourceBuffer, size_t sourceBufferSize);

    uint32 ReadBits(uint32 bits);
    void ReadAlignmentBits();

    uint32 GetBytesRead() const;
    uint32 GetBitsRead() const;
    bool IsOverflowed() const;

private:
    const AccumulatorType* buffer = nullptr;
    uint32 totalBitCount = 0;

    uint32 bitsRead = 0;
    uint32 wordIndex = 0;
    uint32 bitIndex = 0;
    AccumulatorType accum = 0;
    bool overflowed = false;
};

} // namespace DAVA
