#include <UnitTests/UnitTests.h>

#include <Concurrency/Thread.h>
#include <Engine/Engine.h>
#include <FeatureManager/FeatureManager.h>
#include <FileSystem/FileSystem.h>

using namespace DAVA;

DAVA_TESTCLASS (FeatureManagerTest)
{
    DAVA_TEST (InitManager)
    {
        FeatureManager* manager = GetEngineContext()->featureManager;
        manager->InitFromConfig("~res:/FeatureManager/feature_config.yaml");
        manager->Dump();
    }

    DAVA_TEST (GetFeatureValue)
    {
        FeatureManager* manager = GetEngineContext()->featureManager;
        manager->InitFromConfig("~res:/FeatureManager/feature_config.yaml");

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

