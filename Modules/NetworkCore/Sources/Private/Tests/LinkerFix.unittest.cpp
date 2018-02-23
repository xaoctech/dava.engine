#if defined(__DAVAENGINE_LINUX__)
#include "UnitTests/UnitTests.h"
#include <Utils/FpsMeter.h>

/* Ugly fix for clang linker */

using namespace DAVA;

DAVA_TESTCLASS (ClangLinkerFix)
{
    DAVA_TEST (FpsMeterFix)
    {
        FpsMeter fpsMeter;
        TEST_VERIFY(fpsMeter.GetFps() == 0);
    }
};
#endif