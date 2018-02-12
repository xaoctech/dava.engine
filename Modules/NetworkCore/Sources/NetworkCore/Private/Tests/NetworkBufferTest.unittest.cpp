#include "UnitTests/UnitTests.h"

#include "Engine/Engine.h"
#include "Math/Matrix4.h"
#include "Base/Vector.h"

#if !defined(__DAVAENGINE_ANDROID__)

#include "NetworkCore/Private/NetworkBuffer.h"

using namespace DAVA;

DAVA_TESTCLASS (NetworkBufferTest)
{
    DAVA_TEST (CheckUpdate)
    {
        uint32 ticksNum = 3000;
        const uint8 minSize = 4;
        NetworkBuffer<uint32> buffer(minSize, 0);

        for (uint32 i = 1; i <= ticksNum; ++i)
        {
            uint8 size = minSize;
            uint32 value = i - size + 1;
            if (i < minSize)
            {
                size = i + 1;
                value = 0;
            }
            Vector<uint32> rawData(size);
            for (uint32 j = 0; j < size; ++j, ++value)
            {
                rawData[j] = value;
            }
            buffer.Update(rawData, i);
        }
        TEST_VERIFY(buffer.GetSize() == ticksNum);
    }

    DAVA_TEST (CheckPop)
    {
        uint32 ticksNum = 3000;
        const uint8 minSize = 4;
        NetworkBuffer<uint32> buffer(minSize, 0);

        auto popRes = buffer.Pop();
        TEST_VERIFY(popRes.value == nullptr);
        TEST_VERIFY(popRes.frameId == 0);

        for (uint32 i = 1; i <= ticksNum; ++i)
        {
            uint8 size = minSize;
            uint32 value = i - size + 1;
            if (i < minSize)
            {
                size = i + 1;
                value = 0;
            }
            Vector<uint32> rawData(size);
            for (uint32 j = 0; j < size; ++j, ++value)
            {
                rawData[j] = value;
            }
            buffer.Update(rawData, i);
        }
        TEST_VERIFY(buffer.GetSize() == ticksNum);

        for (uint32 i = 1; i <= ticksNum; ++i)
        {
            auto popRes = buffer.Pop();
            TEST_VERIFY(popRes.value != nullptr);
            TEST_VERIFY(*popRes.value == i);
            TEST_VERIFY(popRes.frameId == i);
        }
        TEST_VERIFY(buffer.GetSize() == 0);

        popRes = buffer.Pop();
        TEST_VERIFY(popRes.value != nullptr);
        TEST_VERIFY(*popRes.value == ticksNum);
        TEST_VERIFY(popRes.frameId == 0);
        TEST_VERIFY(buffer.GetSize() == 0);
    }

    DAVA_TEST (CheckFront)
    {
        uint32 ticksNum = 64;
        const uint8 minSize = 3;
        NetworkBuffer<uint32> buffer(minSize, 0);

        for (uint32 i = 1; i <= ticksNum; ++i)
        {
            uint8 size = minSize;
            uint32 value = i - size + 1;
            if (i < minSize)
            {
                size = i + 1;
                value = 0;
            }
            Vector<uint32> rawData(size);
            for (uint32 j = 0; j < size; ++j, ++value)
            {
                rawData[j] = value;
            }
            buffer.Update(rawData, i);

            auto item = buffer.Front();
            if (i <= minSize)
            {
                TEST_VERIFY(item.frameId == 1);
                TEST_VERIFY(*item.value == 1);
            }
            else
            {
                TEST_VERIFY(item.frameId == i - minSize);
                TEST_VERIFY(*item.value == i - minSize);
            }

            buffer.TrimFront();
        }
    }
};

#endif // !defined(__DAVAENGINE_ANDROID__)
