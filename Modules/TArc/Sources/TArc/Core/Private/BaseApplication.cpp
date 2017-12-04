#include "TArc/Core/BaseApplication.h"
#include "TArc/Core/Core.h"
#include "TArc/Testing/TArcTestCore.h"
#include "TArc/DataProcessing/TArcAnyCasts.h"
#include "TArc/SharedModules/ThemesModule/ThemesModule.h"
#include "TArc/Utils/AssertGuard.h"
#include "QtHelpers/RunGuard.h"

#include "Engine/Engine.h"
#include "CommandLine/CommandLineParser.h"

void InitTArcResources()
{
    Q_INIT_RESOURCE(TArcResources);
}

namespace DAVA
{
namespace TArc
{
int BaseApplication::Run()
{
    InitTArcResources();
    Engine e;

    if (!AllowMultipleInstances() && !CommandLineParser::CommandIsFound("--selftest"))
    {
        QtHelpers::RunGuard runGuard(GetInstanceKey());
        if (runGuard.TryToRun())
        {
            return RunImpl();
        }
        else
            return 0;
    }

    return RunImpl();
}

int BaseApplication::RunImpl()
{
    EngineInitInfo initInfo = GetInitInfo();

    // TODO remove this retain after merge with PR-2443
    SafeRetain(initInfo.options.Get());

    Engine* engine = Engine::Instance();

    engine->cleanup.Connect(this, &BaseApplication::Cleanup);

    if (CommandLineParser::CommandIsFound("--selftest"))
    {
        isTestEnv = true;

        SetupToolsAssertHandlers(eApplicationMode::TEST_MODE);
        engine->Init(eEngineRunMode::GUI_EMBEDDED, initInfo.modules, initInfo.options.Get());
        RegisterAnyCasts();
        RegisterEditorAnyCasts();
        RegisterReflectionExtensions();

        const EngineContext* engineContext = engine->GetContext();
        DVASSERT(engineContext);
        Init(engineContext);

        TestCore testCore(*engine);
        return engine->Run();
    }
    else
    {
        SetupToolsAssertHandlers(initInfo.runMode == eEngineRunMode::CONSOLE_MODE ? eApplicationMode::CONSOLE_MODE : eApplicationMode::GUI_MODE);
        engine->Init(initInfo.runMode, initInfo.modules, initInfo.options.Get());
        RegisterAnyCasts();
        RegisterEditorAnyCasts();
        RegisterReflectionExtensions();

        Core core(*engine);
        Init(&core);
        core.PostInit();
        CreateModules(&core);
        return engine->Run();
    }
}

void BaseApplication::Init(const EngineContext* /*engineContext*/)
{
}

void BaseApplication::Init(Core* tarcCore)
{
    DVASSERT(tarcCore != nullptr);
    Init(tarcCore->GetEngineContext());
}

void BaseApplication::RegisterEditorAnyCasts()
{
}

void BaseApplication::RegisterReflectionExtensions()
{
}

void BaseApplication::Cleanup()
{
}

bool BaseApplication::AllowMultipleInstances() const
{
    return true;
}

QString BaseApplication::GetInstanceKey() const
{
    return QString();
}

bool BaseApplication::IsTestEnvironment() const
{
    return isTestEnv;
}

} // namespace TArc
} // namespace DAVA
