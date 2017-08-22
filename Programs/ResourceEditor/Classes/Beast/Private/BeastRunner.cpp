#if defined(__DAVAENGINE_BEAST__)

#include "Classes/Beast/Private/BeastRunner.h"
#include "Classes/Beast/Private/LightmapsPacker.h"

#include "Settings/SettingsManager.h"
#include "Utils/SceneUtils/SceneUtils.h"

#include <Beast/SceneParser.h>
#include <Beast/BeastProxy.h>

#include <Time/SystemTimer.h>
#include <Scene3D/Scene.h>
#include <FileSystem/FileSystem.h>

//Beast
BeastRunner::BeastRunner(BeastProxy* proxy, DAVA::Scene* scene, const DAVA::FilePath& scenePath_, const DAVA::FilePath& outputPath_, eBeastMode mode, std::unique_ptr<DAVA::TArc::WaitHandle> waitHandle_)
    : beastProxy(proxy)
    , workingScene(scene)
    , scenePath(scenePath_)
    , waitHandle(std::move(waitHandle_))
    , outputPath(outputPath_)
    , beastMode(mode)
{
    DVASSERT(beastProxy != nullptr);

    outputPath.MakeDirectoryPathname();
    beastManager = beastProxy->CreateManager();
    beastProxy->SetMode(beastManager, mode);
}

BeastRunner::~BeastRunner()
{
    beastProxy->SafeDeleteManager(&beastManager);
}

void BeastRunner::RunUIMode()
{
    Start();

    while (Process() == false)
    {
        if (cancelledManually)
        {
            beastProxy->Cancel(beastManager);
            break;
        }
        else if (waitHandle)
        {
            waitHandle->SetProgressValue(beastProxy->GetCurTaskProcess(beastManager));
            waitHandle->SetMessage(beastProxy->GetCurTaskName(beastManager).c_str());
            cancelledManually |= waitHandle->WasCanceled();
        }

        Sleep(15);
    }

    Finish(cancelledManually || beastProxy->WasCancelled(beastManager));

    if (waitHandle)
    {
        waitHandle.reset();
    }
}

void BeastRunner::Start()
{
    startTime = DAVA::SystemTimer::GetMs();

    DAVA::FilePath path = GetLightmapDirectoryPath();
    if (beastMode == eBeastMode::MODE_LIGHTMAPS)
    {
        DAVA::FileSystem::Instance()->CreateDirectory(path, false);
        DAVA::FileSystem::Instance()->CreateDirectory(outputPath, true);
    }

    beastProxy->SetLightmapsDirectory(beastManager, path);
    beastProxy->Run(beastManager, workingScene);
}

bool BeastRunner::Process()
{
    beastProxy->Update(beastManager);
    return beastProxy->IsJobDone(beastManager);
}

void BeastRunner::Finish(bool canceled)
{
    if (!canceled && beastMode == eBeastMode::MODE_LIGHTMAPS)
    {
        PackLightmaps();
    }

    DAVA::FileSystem::Instance()->DeleteDirectory(SceneParser::GetTemporaryFolder());
    if (beastMode == eBeastMode::MODE_LIGHTMAPS)
    {
        DAVA::FileSystem::Instance()->DeleteDirectory(GetLightmapDirectoryPath());
    }
}

void BeastRunner::PackLightmaps()
{
    DAVA::FilePath inputDir = GetLightmapDirectoryPath();
    DAVA::FilePath outputDir = outputPath;

    DAVA::FileSystem::Instance()->MoveFile(inputDir + "landscape.png", scenePath.GetDirectory() + "temp_landscape_lightmap.png", true);

    LightmapsPacker packer;
    packer.SetInputDir(inputDir);

    packer.SetOutputDir(outputDir);
    packer.PackLightmaps(DAVA::GPU_ORIGIN);
    packer.CreateDescriptors();
    packer.ParseSpriteDescriptors();

    beastProxy->UpdateAtlas(beastManager, packer.GetAtlasingData());

    DAVA::FileSystem::Instance()->MoveFile(scenePath.GetDirectory() + "temp_landscape_lightmap.png", outputDir + "landscape.png", true);
    DAVA::FileSystem::Instance()->DeleteDirectory(scenePath.GetDirectory() + "$process/");
}

DAVA::FilePath BeastRunner::GetLightmapDirectoryPath() const
{
    return scenePath + "_beast/";
}


#endif //#if defined (__DAVAENGINE_BEAST__)
