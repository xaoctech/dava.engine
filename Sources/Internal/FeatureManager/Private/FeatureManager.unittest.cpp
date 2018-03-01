#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)

#include <UnitTests/UnitTests.h>

#include <Concurrency/Thread.h>
#include <DLCManager/DLCDownloader.h>
#include <Engine/Engine.h>
#include <FeatureManager/FeatureManager.h>
#include <FileSystem/FileSystem.h>
#include <Platform/Process.h>

#include <cstdlib>

using namespace DAVA;

DAVA_TESTCLASS (FeatureManagerTest)
{
    void DownloadConfig()
    {
        auto* fileSystem = GetEngineContext()->fileSystem;

        GetEngineContext()->fileSystem->DeleteFile("~doc:/FeatureManagerTest/feature_config.yaml");

        String enginePath(LOCAL_FRAMEWORK_SOURCE_PATH);
        String startPath = enginePath + "/Programs/FeatureServer/start_server.py";
        String serverPath = enginePath + "/Programs/FeatureServer/feature_server.py";
        String configPath = enginePath + "/Programs/UnitTests/Data/FeatureManager/";

        Process procStart("python", { startPath, serverPath, configPath });
        procStart.Run(false);
        //let server start...
        Thread::Sleep(1000);

        DLCDownloader::Hints hints;
        hints.timeout = 3;
        DLCDownloader* downloader = DLCDownloader::Create(hints);
        SCOPE_EXIT
        {
            DLCDownloader::Destroy(downloader);
        };

        DLCDownloader::ITask* task = downloader->StartTask("localhost:9191/feature_config.yaml", "~doc:/feature_config.yaml");
        SCOPE_EXIT
        {
            downloader->RemoveTask(task);
        };
        downloader->WaitTask(task);
        const DLCDownloader::TaskStatus& status = downloader->GetTaskStatus(task);
        TEST_VERIFY(status.error.errorHappened == false);

        Process procStop("python", { enginePath + "/Programs/FeatureServer/stop_server.py" });
        procStop.Run(false);
        procStop.Wait();
        procStart.Wait();
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
