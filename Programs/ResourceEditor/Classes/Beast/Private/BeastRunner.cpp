#if defined(__DAVAENGINE_BEAST__)

#include "Classes/Beast/Private/BeastRunner.h"
#include "Classes/Beast/Private/LightmapsPacker.h"

#include <REPlatform/Scene/Utils/Utils.h>
#include <REPlatform/Scene/Utils/RETextureDescriptorUtils.h>

#include <Beast/SceneParser.h>

#include <Time/SystemTimer.h>
#include <Scene3D/Scene.h>
#include <FileSystem/FileSystem.h>

//Beast
BeastRunner::BeastRunner(DAVA::Scene* scene, const DAVA::FilePath& scenePath_, const DAVA::FilePath& outputPath_, Beast::eBeastMode mode, std::unique_ptr<DAVA::WaitHandle> waitHandle_)
    : beastManager(new Beast::BeastManager())
    , waitHandle(std::move(waitHandle_))
    , workingScene(scene)
    , scenePath(scenePath_)
    , outputPath(outputPath_)
    , beastMode(mode)
{
    outputPath.MakeDirectoryPathname();
    beastManager->SetMode(beastMode);
}

void BeastRunner::RunUIMode()
{
    Start();

    while (Process() == false)
    {
        if (cancelledManually)
        {
            beastManager->Cancel();
            break;
        }
        else if (waitHandle)
        {
            waitHandle->SetProgressValue(beastManager->GetCurTaskProcess());
            waitHandle->SetMessage(beastManager->GetCurTaskName().c_str());
            cancelledManually |= waitHandle->WasCanceled();
        }

        Sleep(15);
    }

    Finish(cancelledManually || beastManager->WasCancelled());

    if (waitHandle)
    {
        waitHandle.reset();
    }
}

void BeastRunner::Start()
{
    startTime = DAVA::SystemTimer::GetMs();

    DAVA::FilePath path = GetLightmapDirectoryPath();
    if (beastMode == Beast::eBeastMode::MODE_LIGHTMAPS)
    {
        DAVA::FileSystem::Instance()->CreateDirectory(path, false);
        DAVA::FileSystem::Instance()->CreateDirectory(outputPath, true);
    }

    beastManager->SetLightmapsDirectory(path.GetAbsolutePathname());
    beastManager->Run(workingScene);
}

bool BeastRunner::Process()
{
    beastManager->Update();
    return beastManager->IsJobDone();
}

void BeastRunner::Finish(bool canceled)
{
    if (!canceled && beastMode == Beast::eBeastMode::MODE_LIGHTMAPS)
    {
        PackLightmaps();
    }

    DAVA::FileSystem::Instance()->DeleteDirectory(Beast::SceneParser::GetTemporaryFolder());
    if (beastMode == Beast::eBeastMode::MODE_LIGHTMAPS)
    {
        DAVA::FileSystem::Instance()->DeleteDirectory(GetLightmapDirectoryPath());
    }
}

bool PackTextures(const DAVA::FilePath& smFile, const DAVA::FilePath& aoFile, const DAVA::FilePath& outputDir, DAVA::FilePath& outputPath)
{
    DVASSERT(smFile.GetFilename() == aoFile.GetFilename());

    DAVA::ScopedPtr<DAVA::Image> smImage(DAVA::ImageSystem::LoadSingleMip(smFile));
    DAVA::ScopedPtr<DAVA::Image> aoImage(DAVA::ImageSystem::LoadSingleMip(aoFile));

    bool success = false;
    if (smImage.get() && aoImage.get())
    {
        DVASSERT(smImage->GetWidth() == aoImage->GetWidth() && smImage->GetHeight() == aoImage->GetHeight());

        DAVA::ScopedPtr<DAVA::Image> smsaImage(DAVA::Image::Create(smImage->GetWidth(), smImage->GetHeight(), DAVA::PixelFormat::FORMAT_RGBA8888));

        DAVA::uint32* smPtr = reinterpret_cast<DAVA::uint32*>(smImage->GetData());
        DAVA::uint32* aoPtr = reinterpret_cast<DAVA::uint32*>(aoImage->GetData());
        DAVA::uint32* smsaPtr = reinterpret_cast<DAVA::uint32*>(smsaImage->GetData());

        DAVA::uint32 dataSize = smImage->GetDataSize() / 4;
        for (DAVA::uint32 i = 0; i < dataSize; ++i)
            *smsaPtr++ = (0xFF & *smPtr++) | (0xFF & *aoPtr++) << 8 | (0xFF) << 24;

        outputPath = outputDir.GetAbsolutePathname() + smFile.GetBasename() + "_sm_sa.png";
        success = DAVA::ImageSystem::Save(outputPath, smsaImage) == DAVA::eErrorCode::SUCCESS;
    }
    return success;
}

void BeastRunner::PackLightmaps()
{
    DAVA::FilePath inputDir = GetLightmapDirectoryPath();
    DAVA::FilePath outputDir = outputPath;

    bool landscapeProcessed = false;
    DAVA::FilePath landscapeSMPath = inputDir + "/sm/landscape.png";
    DAVA::FilePath landscapeAOPath = inputDir + "/ao/landscape.png";
    DAVA::FilePath landscapeOutputPath;
    DAVA::FilePath landscapeTempPath;
    if (PackTextures(landscapeSMPath, landscapeAOPath, outputDir, landscapeOutputPath))
    {
        landscapeTempPath = scenePath.GetDirectory() + "temp_landscape_lightmap.png";
        DAVA::FileSystem::Instance()->MoveFile(landscapeOutputPath, landscapeTempPath, true);
        DAVA::FileSystem::Instance()->DeleteFile(landscapeSMPath);
        DAVA::FileSystem::Instance()->DeleteFile(landscapeAOPath);
        landscapeProcessed = true;
    }

    LightmapsPacker packer;
    packer.SetInputDir(inputDir);
    packer.SetOutputDir(outputDir);
    packer.PackLightmaps(DAVA::GPU_ORIGIN);
    packer.CreateDescriptors();
    packer.ParseSpriteDescriptors();

    DAVA::FilePath smFolder = outputDir + "/sm/";
    DAVA::FilePath aoFolder = outputDir + "/ao/";

    DAVA::ScopedPtr<DAVA::FileList> smList(new DAVA::FileList(smFolder));
    smList->Sort();

    DAVA::ScopedPtr<DAVA::FileList> aoList(new DAVA::FileList(aoFolder));
    aoList->Sort();

    DVASSERT(smList->GetCount() == aoList->GetCount());

    for (DAVA::uint32 i = 0; i < smList->GetCount(); ++i)
    {
        const DAVA::FilePath& smFile = smList->GetPathname(i);
        const DAVA::FilePath& aoFile = aoList->GetPathname(i);
        if (smFile.IsEqualToExtension(".png") && aoFile.IsEqualToExtension(".png"))
        {
            DAVA::FilePath outputPath;
            if (PackTextures(smFile, aoFile, outputDir, outputPath))
            {
                DAVA::RETextureDescriptorUtils::CreateOrUpdateDescriptor(outputPath);
                DAVA::FileSystem::Instance()->DeleteFile(smFile);
                DAVA::FileSystem::Instance()->DeleteFile(aoFile);
            }
        }
    }

    if (landscapeProcessed)
    {
        DAVA::FileSystem::Instance()->MoveFile(landscapeTempPath, landscapeOutputPath, true);
        DAVA::RETextureDescriptorUtils::CreateOrUpdateDescriptor(landscapeOutputPath);
    }

    DAVA::FileSystem::Instance()->DeleteDirectory(scenePath.GetDirectory() + "$process/");
    DAVA::FileSystem::Instance()->DeleteDirectory(smFolder);
    DAVA::FileSystem::Instance()->DeleteDirectory(aoFolder);

    beastManager->UpdateAtlas(packer.GetAtlasingData());
    beastManager->AssignLandscapeTextures(beastManager.get(), outputDir);
}

DAVA::FilePath BeastRunner::GetLightmapDirectoryPath() const
{
    return scenePath + "_beast/";
}


#endif //#if defined (__DAVAENGINE_BEAST__)
