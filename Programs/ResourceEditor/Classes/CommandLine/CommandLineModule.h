#pragma once

#include <TArc/Core/ConsoleModule.h>

#include <Base/BaseTypes.h>
#include <Base/Result.h>
#include <CommandLine/ProgramOptions.h>
#include <FileSystem/FilePath.h>

class CommandLineModule : public DAVA::ConsoleModule
{
public:
    CommandLineModule(const DAVA::Vector<DAVA::String>& commandLine, const DAVA::String& moduleName);

    int GetExitCode() const override;

protected:
    void PostInit() override;
    eFrameResult OnFrame() override;
    void BeforeDestroyed() override;

    virtual bool PostInitInternal();
    virtual void ShowHelpInternal();

    virtual eFrameResult OnFrameInternal();
    virtual void BeforeDestroyedInternal();

    DAVA::Vector<DAVA::String> commandLine;
    DAVA::ProgramOptions options;

    bool isInitialized = false;
    DAVA::Result result = DAVA::Result::RESULT_SUCCESS;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(CommandLineModule, DAVA::ConsoleModule)
    {
    }
};

inline int CommandLineModule::GetExitCode() const
{
    return (result.type == DAVA::Result::RESULT_SUCCESS ? 0 : -1);
}
