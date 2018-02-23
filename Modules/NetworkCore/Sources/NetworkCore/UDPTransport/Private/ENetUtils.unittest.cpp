#include "UnitTests/UnitTests.h"
#include "Engine/Engine.h"
#include "Base/Exception.h"

#if !defined(__DAVAENGINE_ANDROID__)

#include "NetworkCore/UDPTransport/Private/ENetUtils.h"
using namespace DAVA;

DAVA_TESTCLASS (ENetUtilsTest)
{
    DAVA_TEST (ThrowIfErrorTest)
    {
        int i = 0;
        int* notnullptr = &i;

        ThrowIfENetError(notnullptr, "ALL_RIGHT");
        TEST_VERIFY(notnullptr != nullptr);
        try
        {
            ThrowIfENetError(nullptr, "RAISE_EXCEPTION_NULL_PTR");
            TEST_VERIFY(false);
        }
        catch (DAVA::Exception)
        {
        }

        ThrowIfENetError(0, "ALL_RIGHT");
        try
        {
            ThrowIfENetError(-1, "RAISE_EXCEPTION_NOT_ZERO");
            TEST_VERIFY(false);
        }
        catch (DAVA::Exception)
        {
        }
    }
};

#endif // !defined(__DAVAENGINE_ANDROID__)
