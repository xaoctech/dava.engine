#include "FeatureManager/FeatureManagerUtils.h"

#include "Concurrency/Thread.h"
#include <DLCManager/DLCDownloader.h>

namespace DAVA
{
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_LINUX__)
void FeatureManagerUtils::InitLocalFeatureServer(const String& configFolder)
{
    String enginePath(LOCAL_FRAMEWORK_SOURCE_PATH);
    String startPath = enginePath + "/Programs/FeatureServer/start_server.py";
    String serverPath = enginePath + "/Programs/FeatureServer/feature_server.py";
    String configPath = configFolder;

    procStart = new Process("python", { startPath, serverPath, configPath });
    procStart->Run(false);
    //let server start...
    Thread::Sleep(1000);
}

void FeatureManagerUtils::ShutdownLocalFeatureServer()
{
    String enginePath(LOCAL_FRAMEWORK_SOURCE_PATH);
    Process procStop("python", { enginePath + "/Programs/FeatureServer/stop_server.py" });
    procStop.Run(false);
    procStop.Wait();
    if (procStart)
    {
        procStart->Wait();
        SafeDelete(procStart);
    }
}
#endif

void FeatureManagerUtils::DownloadConfig()
{
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
    DVASSERT(status.error.errorHappened == false);
}
}
