#include "UnitTests/UnitTests.h"

#include "Engine/Engine.h"
#include "Math/Matrix4.h"

#if !defined(__DAVAENGINE_ANDROID__)

#include "NetworkCore/Private/NetworkBucket.h"

using namespace DAVA;

DAVA_TESTCLASS (NetworkBucketTest)
{
    using TestBucket = NetworkBucket<uint32>;

    DAVA_TEST (CheckPush)
    {
        uint32 ticksNum = 64;
        const uint32 minSize = 16;
        const uint32 maxSize = 32;
        TestBucket bucket(minSize, maxSize);

        for (uint32 i = 0; i < ticksNum; ++i)
        {
            bool ret = bucket.Push(i, i);
            TEST_VERIFY(ret);
            TEST_VERIFY(bucket.Push(i, i) == false);

            if (i < maxSize)
            {
                TEST_VERIFY(bucket.GetSize() == i + 1);
            }
            else
            {
                TEST_VERIFY(bucket.GetSize() == maxSize);
            }

            uint32 size = minSize;
            uint32 frameId = 0;
            const uint32* raw = bucket.GetRawData(size, frameId);
            TEST_VERIFY(frameId == i);
            for (uint8 j = 0; j < size; ++j)
            {
                if (i < size)
                {
                    TEST_VERIFY(raw[j] == j);
                }
                else
                {
                    TEST_VERIFY(raw[j] == i - size + j + 1);
                }
            }
        }
    }
};

#endif // !defined(__DAVAENGINE_ANDROID__)
