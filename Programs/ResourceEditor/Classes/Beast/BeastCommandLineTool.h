#pragma once

#if defined(__DAVAENGINE_BEAST__)

#include <REPlatform/Global/CommandLineModule.h>

#include <FileSystem/FilePath.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
class Scene;
}

class BeastRunner;
class BeastCommandLineTool : public DAVA::CommandLineModule
{
public:
    BeastCommandLineTool(const DAVA::Vector<DAVA::String>& commandLine);

private:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;
    void ShowHelpInternal() override;

    DAVA::FilePath scenePathname;
    DAVA::FilePath outputPathname;

    BeastRunner* beastRunner = false;
    DAVA::Scene* scene = nullptr;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(BeastCommandLineTool, DAVA::CommandLineModule)
    {
        DAVA::ReflectionRegistrator<BeastCommandLineTool>::Begin()[DAVA::M::CommandName("-beast")]
        .ConstructorByPointer<DAVA::Vector<DAVA::String>>()
        .End();
    }
};

#endif //#if defined (__DAVAENGINE_BEAST__)
