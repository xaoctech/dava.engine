#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
    Class to work with bitstreams
*/
class Bitstream
{
public:
    Bitstream(void* bitstreamPointer, uint32 bitstreamLength);

    /** Reset bitstream to start position */
    void Reset();

    /** Return bitstream length in bytes */
    uint32 Length() const;

    /** Get current bit position */
    uint32 GetPosition() const;

    /** Skip bits */
    void Skip(uint32 bitCount = 1);

    /** Skip bits in writing mode */
    void Forward(uint32 bitCount = 1);

    /** Read bitCount bits from stream */
    uint32 ReadBits(uint32 bitCount = 1);

    /** Show next bitCount bits from stream */
    uint32 ShowBits(uint32 bitCount = 1);

    /** Write lower bitCount bits from bitValue to stream */
    void WriteBits(uint32 bitValue, uint32 bitCount);

    /** Write 1 bit to stream */
    void WriteBit(uint32 bitValue);

    /** Align stream to byte boundary */
    void AlignToByte();

private:
    // Bitstream pointer
    uint32* head = nullptr;
    uint32* tail = nullptr;
    uint32 position = 0;
    uint32 length = 0;
    uint32 bufferA = 0; // Read Buffer [0]
    uint32 bufferB = 0; // Read Buffer [1]
    uint32 writeBuffer = 0; // Write Buffer
};

} // namespace DAVA
