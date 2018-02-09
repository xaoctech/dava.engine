#include "UnitTests/UnitTests.h"

#include <Base/BitWriter.h>
#include <Base/BitReader.h>
#include <Logger/Logger.h>
#include <Utils/StringFormat.h>

using namespace DAVA;

DAVA_TESTCLASS (BitstreamTest)
{
    struct TestItem
    {
        uint32 w;
        uint32 bits;
        uint32 r;
    };

    DAVA_TEST (Test)
    {
        uint8 buffer[32];

        TestItem test[] = {
            { 1, 1 }, // 1 bit
            { 0b1010, 4 }, // 5 bits
            { 0b1100, 4 }, // 9 bits
            { 0b0110, 4 }, // 13 bits
            { 0b0101'1100, 7 }, // 20 bits
            { 0b1101'1001, 8 }, // 28 bits
            { 0b1010, 4 }, // 32 bits - on uint32 boundary
            { 0xA1, 8 }, // 40 bits
            { 0xC6D9, 16 }, // 56 bits
            { 0x5055, 15 }, // 71 bits - cross uint32 boundary
            { 0x01ABCDEF, 25 }, // 96 bits - on uint32 boundary
            { 0x12345678, 32 }, // 128 bits
            { 0b0010, 3 }, // 131 bits
            { 0xFEDCBA55, 32 }, // 163 bits
        };

        BitWriter w(buffer, 32);
        for (const TestItem& i : test)
        {
            w.WriteBits(i.w, i.bits);
        }
        w.Flush();

        uint32 bytesWritten = w.GetBytesWritten();
        uint32 bitsWritten = w.GetBitsWritten();
        TEST_VERIFY(bitsWritten == 163);
        TEST_VERIFY(bytesWritten == 21);

        BitReader r(buffer, 32);
        for (TestItem& i : test)
        {
            uint32 v = r.ReadBits(i.bits);
            i.r = v;
        }

        uint32 bytesRead = r.GetBytesRead();
        uint32 bitsRead = r.GetBitsRead();
        TEST_VERIFY(bitsRead == 163);
        TEST_VERIFY(bytesRead == 21);

        for (const TestItem& i : test)
        {
            TEST_VERIFY_WITH_MESSAGE(i.r == i.w, Format("expected=%08X, read=%08X, bits=%u", i.w, i.r, i.bits));
        }

        // test second read from the same buffer
        BitReader r2(buffer, 32);
        for (TestItem& i : test)
        {
            uint32 v = r2.ReadBits(i.bits);
            i.r = v;
        }

        uint32 bytesRead2 = r2.GetBytesRead();
        uint32 bitsRead2 = r2.GetBitsRead();
        TEST_VERIFY(bitsRead2 == bitsRead);
        TEST_VERIFY(bytesRead2 == bytesRead);

        for (const TestItem& i : test)
        {
            TEST_VERIFY_WITH_MESSAGE(i.r == i.w, Format("expected=%08X, read=%08X, bits=%u", i.w, i.r, i.bits));
        }
    }

    DAVA_TEST (TestOverflow)
    {
        uint8 buffer[12];

        BitWriter w(buffer, 4);
        w.WriteBits(10, 16);
        TEST_VERIFY(w.IsOverflowed() == false);
        w.WriteBits(20, 12);
        TEST_VERIFY(w.IsOverflowed() == false);
        w.WriteBits(30, 8);
        TEST_VERIFY(w.IsOverflowed() == true);
        w.Flush();

        uint32 v = 0;
        BitReader r(buffer, 4);
        v = r.ReadBits(16);
        TEST_VERIFY(r.IsOverflowed() == false);
        TEST_VERIFY(v == 10);
        v = r.ReadBits(12);
        TEST_VERIFY(r.IsOverflowed() == false);
        TEST_VERIFY(v == 20);
        v = r.ReadBits(8);
        TEST_VERIFY(r.IsOverflowed() == true);
        TEST_VERIFY(v == 0);
    }

    DAVA_TEST (TestRewind)
    {
        uint8 buffer[20] = {};

        BitWriter w(buffer, 20);
        w.WriteBits(0b101'1011'1101, 11);
        TEST_VERIFY(w.GetBitsWritten() == 11);
        TEST_VERIFY(w.GetBytesWritten() == 2);
        w.Rewind(5);
        TEST_VERIFY(w.GetBitsWritten() == 6);
        TEST_VERIFY(w.GetBytesWritten() == 1);
        w.Flush();
        {
            BitReader r(buffer, 20);
            uint32 v = r.ReadBits(6);
            TEST_VERIFY(v == 0b11'1101);
        }
        w.WriteBits(0b1001, 4);
        TEST_VERIFY(w.GetBitsWritten() == 10);
        TEST_VERIFY(w.GetBytesWritten() == 2);
        w.Flush();
        {
            BitReader r(buffer, 20);
            uint32 v = r.ReadBits(6);
            TEST_VERIFY(v == 0b11'1101);
            v = r.ReadBits(4);
            TEST_VERIFY(v == 0b1001);
        }
        w.Rewind(w.GetBitsWritten());
        TEST_VERIFY(w.GetBitsWritten() == 0);
        TEST_VERIFY(w.GetBytesWritten() == 0);

        w.WriteBits(0b0110, 4);
        w.WriteBits(0xAAAAAAAA, 32);
        w.WriteBits(0xBBBBBBBB, 32);
        w.WriteBits(0b101, 3);
        w.Rewind(3);
        w.WriteBits(0b010, 3);
        w.Flush();
        TEST_VERIFY(w.GetBitsWritten() == 71);
        TEST_VERIFY(w.GetBytesWritten() == 9);
        {
            BitReader r(buffer, 20);
            uint32 v = r.ReadBits(4);
            TEST_VERIFY(v == 0b0110);
            v = r.ReadBits(32);
            TEST_VERIFY(v == 0xAAAAAAAA);
            v = r.ReadBits(32);
            TEST_VERIFY(v == 0xBBBBBBBB);
            v = r.ReadBits(3);
            TEST_VERIFY(v == 0b010);
        }

        w.Rewind(67);
        w.WriteBits(0b1'0111, 5);
        w.Flush();
        TEST_VERIFY(w.GetBitsWritten() == 9);
        TEST_VERIFY(w.GetBytesWritten() == 2);
        {
            BitReader r(buffer, 20);
            uint32 v = r.ReadBits(4);
            TEST_VERIFY(v == 0b0110);
            v = r.ReadBits(5);
            TEST_VERIFY(v == 0b1'0111);
        }
    }

    DAVA_TEST (TestPatching)
    {
        uint8 buffer[32];
        BitWriter w(buffer, 32);

        w.WriteBits(0x09, 4); // 4
        w.WriteBits(0x04AB12, 20); // 24
        w.WriteBits(0xAABB, 16); // 40
        w.WriteBits(0xDABEEF, 24); // 64
        w.WriteBits(0xAA55AA55, 32); // 96
        w.WriteBits(0xC6, 8); // 104

        w.PatchBits(0, ~0x09, 4);
        w.PatchBits(4, ~0x04AB12, 20);
        w.PatchBits(24, ~0xAABB, 16);
        w.PatchBits(40, ~0xDABEEF, 24);
        w.PatchBits(64, ~0xAA55AA55, 32);
        w.PatchBits(96, ~0xC6, 8);

        w.WriteAlignmentBits();
        w.Flush();

        BitReader r(buffer, 32);
        uint32 v1 = r.ReadBits(4);
        TEST_VERIFY(v1 == (~0x09 & 0x0F));
        uint32 v2 = r.ReadBits(20);
        TEST_VERIFY(v2 == (~0x04AB12 & 0x0FFFFF));
        uint32 v3 = r.ReadBits(16);
        TEST_VERIFY(v3 == (~0xAABB & 0xFFFF));
        uint32 v4 = r.ReadBits(24);
        TEST_VERIFY(v4 == (~0xDABEEF & 0xFFFFFF));
        uint32 v5 = r.ReadBits(32);
        TEST_VERIFY(v5 == ~0xAA55AA55);
        uint32 v6 = r.ReadBits(8);
        TEST_VERIFY(v6 == (~0xC6 & 0xFF));
    }
};
