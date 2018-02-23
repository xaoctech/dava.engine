#include "UnitTests/UnitTests.h"

#include <Engine/Engine.h>
#include <Math/Quaternion.h>
#include <Time/SystemTimer.h>

#if !defined(__DAVAENGINE_ANDROID__)

using namespace DAVA;

DAVA_TESTCLASS (NetworkInputSystemTest)
{
    DAVA_TEST (CheckPackUnpackQuaternionSimple)
    {
        Quaternion src;
        uint64 pack = src.Pack();
        Quaternion dst;
        dst.Unpack(pack);
        TEST_VERIFY(src == dst);
    }

    DAVA_TEST (CheckPackUnpackQuaternionCustom)
    {
        int64 ts = SystemTimer::GetMs();
        for (int32 i = -360; i <= 360; ++i)
        {
            Quaternion src;
            float32 f = static_cast<float32>(i);
            src.ConstructRotationFastX(DegToRad(f));
            src.ConstructRotationFastY(DegToRad(f));
            src.ConstructRotationFastZ(DegToRad(f));
            src.Normalize();

            uint64 pack = src.Pack();
            Quaternion dst;
            dst.Unpack(pack);
            for (int32 p = 0; p < 4; ++p)
            {
                // please see: https://engineering.riotgames.com/news/compressing-skeletal-animation-data
                TEST_VERIFY(std::fabs(src.data[p] - dst.data[p]) < 0.00004314);
            }
        }
    }
};

#endif // !defined(__DAVAENGINE_ANDROID__)
