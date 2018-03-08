#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_LINUX__)

#include <UnitTests/UnitTests.h>

#include <Concurrency/Thread.h>
#include <Engine/Engine.h>
#include <FeatureManager/FeatureManager.h>
#include <FeatureManager/FeatureManagerUtils.h>
#include <FileSystem/FileSystem.h>

using namespace DAVA;

DAVA_TESTCLASS (FeatureManagerTest)
{
    void DownloadConfig()
    {
        GetEngineContext()->fileSystem->DeleteFile("~doc:/FeatureManagerTest/feature_config.yaml");

        FeatureManagerUtils* featureManagerUtils = GetEngineContext()->featureManagerUtils;
        featureManagerUtils->InitLocalFeatureServer(String(LOCAL_FRAMEWORK_SOURCE_PATH) + "/Programs/UnitTests/Data/FeatureManager/");
        featureManagerUtils->DownloadConfig();
        featureManagerUtils->ShutdownLocalFeatureServer();
    }

    DAVA_TEST (InitManager)
    {
        DownloadConfig();
        FeatureManager* manager = GetEngineContext()->featureManager;
        manager->InitFromConfig("~doc:/feature_config.yaml");
        manager->Dump();
    }

    DAVA_TEST (GetFeatureValue)
    {
        DownloadConfig();
        FeatureManager* manager = GetEngineContext()->featureManager;
        manager->InitFromConfig("~doc:/feature_config.yaml");

        Any f1 = manager->GetFeatureValue(FastName("feature1"));
        TEST_VERIFY(f1.IsEmpty() == false && f1.Get<bool>() == true);
        Any f2 = manager->GetFeatureValue(FastName("feature2"));
        TEST_VERIFY(f2.IsEmpty() == false && f2.Get<bool>() == false);
        Any f3 = manager->GetFeatureValue(FastName("feature3"));
        TEST_VERIFY(f3.IsEmpty() == false && f3.Get<bool>() == true);
        Any f4 = manager->GetFeatureValue(FastName("feature4"));
        TEST_VERIFY(f4.IsEmpty() == true);

        int32 testInt = 0;

        DAVA_IF_FEATURE(feature1)
        {
            testInt++;
        }
        TEST_VERIFY(testInt == 1);
        DAVA_IF_FEATURE(feature2)
        {
            testInt++;
        }
        TEST_VERIFY(testInt == 1);
        DAVA_IF_FEATURE(feature3)
        {
            testInt++;
        }
        TEST_VERIFY(testInt == 2);

        bool wasException = false;
        try
        {
            DAVA_IF_FEATURE(feature4)
            {
                testInt++;
            }
        }
        catch (const Exception&)
        {
            wasException = true;
        }
        TEST_VERIFY(wasException == true);
        TEST_VERIFY(testInt == 2);
    }
};

#endif
