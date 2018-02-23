#include "Engine/Engine.h"

#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/EngineBackend.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Base/GlobalEnum.h"
#include "FileSystem/KeyedArchive.h"

ENUM_DECLARE(DAVA::eEngineRunMode)
{
    ENUM_ADD_DESCR(DAVA::eEngineRunMode::GUI_STANDALONE, "Standalone");
    ENUM_ADD_DESCR(DAVA::eEngineRunMode::GUI_EMBEDDED, "Embedded");
    ENUM_ADD_DESCR(DAVA::eEngineRunMode::CONSOLE_MODE, "Console");
}

namespace DAVA
{
namespace EngineSingletonNamespace
{
Engine* engineSingleton = nullptr;
}

DAVA_REFLECTION_IMPL(Engine)
{
    ReflectionRegistrator<Engine>::Begin()
    .Field("getRunMode", &Engine::GetRunMode, nullptr)[M::EnumT<eEngineRunMode>()]
    .Field("isStandaloneGUIMode", &Engine::IsStandaloneGUIMode, nullptr)
    .Field("isEmbeddedGUIMode", &Engine::IsEmbeddedGUIMode, nullptr)
    .Field("isConsoleMode", &Engine::IsConsoleMode, nullptr)
    .Field("isSuspended", &Engine::IsSuspended, nullptr)
    .Field("options", &Engine::GetOptions, nullptr)
    .Field("commandLine", &Engine::GetCommandLine, nullptr)
    .Method("quitAsync", &Engine::QuitAsync)
    .Method("terminate", &Engine::Terminate)
    .End();
}

Engine* Engine::Instance()
{
    return EngineSingletonNamespace::engineSingleton;
}

Engine::Engine()
{
    DVASSERT(EngineSingletonNamespace::engineSingleton == nullptr);

    EngineSingletonNamespace::engineSingleton = this;

    engineBackend = Private::EngineBackend::Instance();
    engineBackend->EngineCreated(this);
}

Engine::~Engine()
{
    engineBackend->EngineDestroyed();
    engineBackend = nullptr;

    EngineSingletonNamespace::engineSingleton = nullptr;
}

const EngineContext* Engine::GetContext() const
{
    return engineBackend->GetContext();
}

Window* Engine::PrimaryWindow() const
{
    return engineBackend->GetPrimaryWindow();
}

const Vector<Window*>& Engine::GetWindows() const
{
    return engineBackend->GetWindows();
}

eEngineRunMode Engine::GetRunMode() const
{
    return engineBackend->GetRunMode();
}

bool Engine::IsStandaloneGUIMode() const
{
    return engineBackend->IsStandaloneGUIMode();
}

bool Engine::IsEmbeddedGUIMode() const
{
    return engineBackend->IsEmbeddedGUIMode();
}

bool Engine::IsConsoleMode() const
{
    return engineBackend->IsConsoleMode();
}

void Engine::Init(eEngineRunMode runMode, const Vector<String>& modules, KeyedArchive* options)
{
    engineBackend->Init(runMode, modules, options);
}

int Engine::Run()
{
    return engineBackend->Run();
}

void Engine::QuitAsync(int exitCode)
{
    engineBackend->Quit(exitCode);
}

void Engine::Terminate(int exitCode)
{
    engineBackend->Terminate(exitCode);
}

void Engine::SetCloseRequestHandler(const Function<bool(Window*)>& handler)
{
    engineBackend->SetCloseRequestHandler(handler);
}

uint32 Engine::GetGlobalFrameIndex() const
{
    return engineBackend->GetGlobalFrameIndex();
}

const Vector<String>& Engine::GetCommandLine() const
{
    return engineBackend->GetCommandLine();
}

Vector<char*> Engine::GetCommandLineAsArgv() const
{
    return engineBackend->GetCommandLineAsArgv();
}

const KeyedArchive* Engine::GetOptions() const
{
    return engineBackend->GetOptions();
}

bool Engine::IsSuspended() const
{
    return engineBackend->IsSuspended();
}

void Engine::SetScreenTimeoutEnabled(bool enabled)
{
    engineBackend->SetScreenTimeoutEnabled(enabled);
}

} // namespace DAVA
