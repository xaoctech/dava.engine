#include "UnitTests/UnitTests.h"

#include "Engine/Engine.h"

#if !defined(__DAVAENGINE_ANDROID__)

#include "NetworkCore/NetworkCoreModule.h"

using namespace DAVA;

DAVA_TESTCLASS (NetworkCoreTest)
{
    DAVA_TEST (CheckStatus)
    {
        const ModuleManager& moduleManager = *GetEngineContext()->moduleManager;
        NetworkCoreModule* networkCore = moduleManager.GetModule<NetworkCoreModule>();
    }
};

#endif // !defined(__DAVAENGINE_ANDROID__)
