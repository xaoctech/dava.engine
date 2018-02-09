#include "Classes/Beast/BeastCommandLineTool.h"

#if defined(__DAVAENGINE_BEAST__)

#include "Classes/Beast/Private/BeastRunner.h"

#include "Classes/CommandLine/Private/OptionName.h"
#include "Classes/CommandLine/Private/SceneConsoleHelper.h"

#include <REPlatform/Scene/Utils/Utils.h>

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Beast/BeastConstants.h>

#include <Base/ScopedPtr.h>
#include <Logger/Logger.h>
#include <Scene3D/Scene.h>

BeastCommandLineTool::BeastCommandLineTool(const DAVA::Vector<DAVA::String>& commandLine)
    : CommandLineModule(commandLine, "-beast")
{
    options.AddOption(OptionName::File, DAVA::VariantType(DAVA::String("")), "Full pathname of scene for beasting");
    options.AddOption(OptionName::Output, DAVA::VariantType(DAVA::String("")), "Full path for output folder for beasting");
    options.AddOption(OptionName::QualityConfig, DAVA::VariantType(DAVA::String("")), "Full path for quality.yaml file");
}

bool BeastCommandLineTool::PostInitInternal()
{
    scenePathname = options.GetOption(OptionName::File).AsString();
    if (scenePathname.IsEmpty() || !scenePathname.IsEqualToExtension(".sc2"))
    {
        DAVA::Logger::Error("Scene was not selected");
        return false;
    }

    outputPathname = options.GetOption(OptionName::Output).AsString();
    if (outputPathname.IsEmpty())
    {
        DAVA::Logger::Error("Out folder was not selected");
        return false;
    }
    else
    {
        outputPathname.MakeDirectoryPathname();
    }

    bool qualityInitialized = SceneConsoleHelper::InitializeQualitySystem(options, scenePathname);
    if (!qualityInitialized)
    {
        DAVA::Logger::Error("Cannot create path to quality.yaml from %s", scenePathname.GetAbsolutePathname().c_str());
        return false;
    }

    scene = new DAVA::Scene();
    if (scene->LoadScene(scenePathname) == DAVA::SceneFileV2::eError::ERROR_NO_ERROR)
    {
        scene->Update(0.1f);

        beastRunner = new BeastRunner(scene, scenePathname, outputPathname, Beast::eBeastMode::MODE_LIGHTMAPS, std::unique_ptr<DAVA::WaitHandle>());
        beastRunner->Start();
    }
    else
    {
        SafeRelease(scene);
        return false;
    }

    return true;
}

DAVA::ConsoleModule::eFrameResult BeastCommandLineTool::OnFrameInternal()
{
    if (scene != nullptr && beastRunner != nullptr)
    {
        bool finished = beastRunner->Process();
        if (finished == false)
        {
            return DAVA::ConsoleModule::eFrameResult::CONTINUE;
        }
    }

    return DAVA::ConsoleModule::eFrameResult::FINISHED;
}

void BeastCommandLineTool::BeforeDestroyedInternal()
{
    if (beastRunner)
    {
        beastRunner->Finish(false);
    }

    if (scene != nullptr)
    {
        scene->SaveScene(scenePathname, false);
        DAVA::SafeRelease(scene);
    }

    DAVA::SafeDelete(beastRunner);
    SceneConsoleHelper::FlushRHI();
}

void BeastCommandLineTool::ShowHelpInternal()
{
    CommandLineModule::ShowHelpInternal();

    DAVA::Logger::Info("Examples:");
    DAVA::Logger::Info("\t-beast -file /Users/SmokeTest/DataSource/3d/Maps/scene.sc2 -output /Users/SmokeTest/DataSource/3d/Maps/beast");
}

DECL_TARC_MODULE(BeastCommandLineTool);

#endif //#if defined (__DAVAENGINE_BEAST__)
