#include "TestClient.h"
#include "Battle.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "Flow/FlowBattleController.h"
#include "Flow/FlowBattleService.h"
#include "GameClient.h"

#include "Game.h"

#include <Base/FastName.h>
#include <CommandLine/CommandLineParser.h>
#include <Concurrency/Thread.h>
#include <Debug/DVAssertDefaultHandlers.h>
#include <Debug/ProfilerCPU.h>
#include <Debug/ProfilerGPU.h>
#include <Debug/ProfilerOverlay.h>
#include <Debug/DebugOverlay.h>
#include <Debug/Private/ImGui.h>
#include <DeviceManager/DeviceManager.h>
#include <FileSystem/FileSystem.h>
#include <Engine/Engine.h>
#include <Input/Keyboard.h>
#include <Logger/Logger.h>
#include <MemoryManager/MemoryLogger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Scene3D/Scene.h>
#include <Time/SystemTimer.h>
#include <UI/DefaultUIPackageBuilder.h>
#include <UI/Flow/UIFlowContext.h>
#include <UI/Flow/UIFlowStateComponent.h>
#include <UI/Flow/UIFlowStateSystem.h>
#include <UI/UIControlSystem.h>
#include <UI/UIPackageLoader.h>
#include <UI/UIScreen.h>
#include <UI/UIYamlLoader.h>
#include <Utils/Random.h>

#if defined(__DAVAENGINE_LINUX__) || defined(__DAVAENGINE_MACOS__)
#include <signal.h>
#endif

int DAVAMain(DAVA::Vector<DAVA::String> cmdline)
{
    using namespace DAVA;

    Assert::SetupDefaultHandlers();

    Engine e;

    String host = CommandLineParser::GetCommandParam("--host");
    if (host.empty())
    {
        host = "localhost";
    }
    String portStr = CommandLineParser::GetCommandParam("--port");
    uint16 port = 9000;
    if (!portStr.empty())
    {
        port = std::stoi(portStr);
    }
    String slowDownFactorStr = CommandLineParser::GetCommandParam("--slow-down");
    float32 slowDownFactor = 0.01f;
    if (!slowDownFactorStr.empty())
    {
        slowDownFactor = std::stof(slowDownFactorStr);
    }
    String freqHzStr = CommandLineParser::GetCommandParam("--hz");
    uint32 freqHz = 60;
    if (!freqHzStr.empty())
    {
        freqHz = std::stoi(freqHzStr);
    }

    String playerKindStr = CommandLineParser::GetCommandParam("--bot");
    PlayerKind playerKind(playerKindStr);

    bool isProfile = CommandLineParser::CommandIsFound("--profile");
    bool isDebug = CommandLineParser::CommandIsFound("--debug");
    bool isLog = CommandLineParser::CommandIsFound("--log");

    Vector<String> modules{
        "JobManager",
        "SoundSystem",
        "LocalizationSystem" // need for any font to work (TODO fix it)
    };

    RegisterGameComponents();

    if (playerKind.IsBot())
    {
#if defined(__DAVAENGINE_LINUX__) || defined(__DAVAENGINE_MACOS__)
        signal(SIGINT, [](int) {
            Engine::Instance()->QuitAsync(0);
        });
#endif
        KeyedArchive* appOptions = new KeyedArchive();
        appOptions->SetInt32("renderer", rhi::RHI_NULL_RENDERER);
        e.Init(eEngineRunMode::CONSOLE_MODE, modules, appOptions);
    }
    else
    {
        e.Init(eEngineRunMode::GUI_STANDALONE, modules, TestClient::CreateOptions());
    }

    String tokenStr = CommandLineParser::GetCommandParam("--token");
    uint64 token1 = Random::Instance()->Rand(60) + 1;
    uint64 token2 = Random::Instance()->Rand(1) + 1;
    uint64 tokenInt = token1 * 10 + token2;
    if (!tokenStr.empty())
    {
        tokenInt = std::stoi(tokenStr);
    }
    FastName token = FastName(Format("%064d", tokenInt));
    Logger::Debug("Token: %s", token.c_str());

    if (isLog)
    {
        String logFileName = Format("client-%llu.log", SystemTimer::GetSystemUptimeUs());
        if (playerKind.IsBot())
        {
            logFileName = Format("bot-%llu-%llu.log", tokenInt, SystemTimer::GetSystemUptimeUs());
        }

        FilePath logFilePath = FileSystem::Instance()->GetCurrentExecutableDirectory() + logFileName;
        GetEngineContext()->logger->SetLogPathname(logFilePath);
    }
    KeyedArchive* clientOptions = new KeyedArchive();
    clientOptions->SetString("hostName", host);
    clientOptions->SetUInt32("port", port);
    clientOptions->SetUInt32("playerKind", playerKind.AsUInt32());
    clientOptions->SetFastName("token", token);
    clientOptions->SetBool("isProfile", isProfile);
    clientOptions->SetBool("isDebug", isDebug);
    clientOptions->SetFloat("slowDownFactor", slowDownFactor);
    clientOptions->SetUInt32("freqHz", freqHz);
    TestClient app(e, clientOptions);
    return e.Run();
}

TestClient::TestClient(DAVA::Engine& engine, DAVA::KeyedArchive* clientOptions)
    : engine(engine)
    , clientOptions(clientOptions)
{
    using namespace DAVA;

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(FlowBattleController);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(FlowBattleService);

    engine.gameLoopStarted.Connect(this, &TestClient::OnLoopStarted);
    engine.windowCreated.Connect(this, &TestClient::OnWindowCreated);
    engine.gameLoopStopped.Connect(this, &TestClient::OnLoopStopped);
    engine.cleanup.Connect(this, &TestClient::OnEngineCleanup);

    engine.suspended.Connect(this, &TestClient::OnSuspended);
    engine.resumed.Connect(this, &TestClient::OnResumed);
    engine.update.Connect(this, &TestClient::OnUpdate);
}

void TestClient::OnLoopStarted()
{
    using namespace DAVA;

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    memoryLogger = std::make_unique<MemoryLogger>();
#endif

    Logger::Debug("****** TestClient::OnLoopStarted");

    PlayerKind playerKind(clientOptions->GetUInt32("playerKind"));
    bool isProfile = clientOptions->GetBool("isProfile");

    if (isProfile)
    {
        ProfilerCPU::globalProfiler->Start();
        ProfilerGPU::globalProfiler->Start();
        if (!playerKind.IsBot())
        {
            ProfilerOverlay::globalProfilerOverlay->SetInputEnabled(true);
        }
    }

    if (playerKind.IsBot())
    {
        botBattle = std::make_unique<Battle>();
        BattleOptions battleOptions = BattleOptions::FromKeyedArchive(clientOptions);
        botBattle->Initialize(battleOptions);
    }
}

void TestClient::OnWindowCreated(DAVA::Window* w)
{
    using namespace DAVA;
    Logger::Debug("****** TestClient::OnWindowCreated");

    DebugInit();

    engine.PrimaryWindow()->update.Connect(this, &TestClient::OnWindowUpdate);
    engine.PrimaryWindow()->draw.Connect(this, &TestClient::Draw);
    VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;

    const Size2i& physicalSize = vcs->GetPhysicalScreenSize();
    float32 screenAspect = static_cast<float32>(physicalSize.dx) / static_cast<float32>(physicalSize.dy);

    const Size2f windowSize = { 1024.f, 1024.f / screenAspect };
    String title = Format("DAVA Engine - TestClient | [%u bit]",
                          static_cast<uint32>(sizeof(pointer_size) * 8));
    w->SetTitleAsync(title);
    w->SetSizeAsync(windowSize);
    w->SetVirtualSize(windowSize.dx, windowSize.dy);

    vcs->RegisterAvailableResourceSize(static_cast<int32>(windowSize.dx),
                                       static_cast<int32>(windowSize.dy),
                                       "Gfx");

    // Init UI
    UIControlSystem* controlSys = w->GetUIControlSystem();
    if (controlSys)
    {
        UIYamlLoader::LoadFonts("~res:/Fonts/Configs/fonts.yaml");

        // Init Flow
        UIFlowStateSystem* stateSys = controlSys->GetSystem<UIFlowStateSystem>();
        if (stateSys)
        {
            stateSys->GetContext()->GetData()->SetArchive("clientOptions", clientOptions);

            RefPtr<UIScreen> screen(new UIScreen());
            controlSys->SetScreen(screen.Get());
            {
                DAVA::DefaultUIPackageBuilder pkgBuilder;
                DAVA::UIPackageLoader().LoadPackage("~res:/UI/Flow.yaml", &pkgBuilder);
                UIControl* flow = pkgBuilder.GetPackage()->GetControls().at(0);
                stateSys->SetFlowRoot(flow);
            }
            {
                DAVA::DefaultUIPackageBuilder pkgBuilder;
                DAVA::UIPackageLoader().LoadPackage("~res:/UI/Root.yaml", &pkgBuilder);
                UIControl* uiRoot = pkgBuilder.GetPackage()->GetControls().at(0);
                screen->AddControl(uiRoot);
            }

            stateSys->ActivateState(stateSys->FindStateByPath("MainMenu"), false);
        }
    }
}

void TestClient::Draw(DAVA::Window* w)
{
}

void TestClient::OnLoopStopped()
{
    using namespace DAVA;
    Logger::Debug("****** TestClient::OnLoopStopped");
}

void TestClient::OnEngineCleanup()
{
    using namespace DAVA;
    Logger::Debug("****** TestClient::OnEngineCleanup");

    botBattle.reset();

    UIControlSystem* controlSys = GetEngineContext()->uiControlSystem;
    if (controlSys)
    {
        UIFlowStateSystem* stateSys = controlSys->GetSystem<UIFlowStateSystem>();
        if (stateSys)
        {
            stateSys->SetFlowRoot(nullptr);
        }
        controlSys->Reset();
    }
}

void TestClient::OnSuspended()
{
    using namespace DAVA;
    Logger::Debug("****** TestClient::OnSuspended");
}

void TestClient::OnResumed()
{
    using namespace DAVA;
    Logger::Debug("****** TestClient::OnResumed");
}

void TestClient::OnUpdate(DAVA::float32 frameDelta)
{
    using namespace DAVA;
    DAVA_PROFILER_CPU_SCOPE("TestClient::OnUpdate");

    if (botBattle)
    {
        botBattle->Update(frameDelta);
    }

    DebugUpdate();
}

void TestClient::OnWindowUpdate(DAVA::Window* window, DAVA::float32 frameDelta)
{
}

DAVA::KeyedArchive* TestClient::CreateOptions()
{
    using namespace DAVA;
    KeyedArchive* appOptions = new KeyedArchive();
    appOptions->SetInt32("shader_const_buffer_size", 4 * 1024 * 1024);
    appOptions->SetInt32("max_index_buffer:_count", 3 * 1024);
    appOptions->SetInt32("max_vertex_buffer_count", 3 * 1024);
    appOptions->SetInt32("max_const_buffer_count", 16 * 1024);
    appOptions->SetInt32("max_texture_count", 2048);
    appOptions->SetInt32("max_texture_set_count", 2048);
    appOptions->SetInt32("max_sampler_state_count", 128);
    appOptions->SetInt32("max_pipeline_state_count", 1024);
    appOptions->SetInt32("max_depthstencil_state_count", 256);
    appOptions->SetInt32("max_render_pass_count", 64);
    appOptions->SetInt32("max_command_buffer_count", 64);
    appOptions->SetInt32("max_packet_list_count", 64);
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    appOptions->SetInt32("rhi_threaded_frame_count", 2);
    return appOptions;
}

void TestClient::DebugInit()
{
    ImGui::Initialize();
}

void TestClient::DebugUpdate()
{
    using namespace DAVA;
    const EngineContext* context = GetEngineContext();
    Keyboard* kb = context->deviceManager->GetKeyboard();
    if (kb != nullptr)
    {
        DigitalElementState keyState = kb->GetKeyState(DAVA::eInputElements::KB_GRAVE);
        if (keyState.IsJustPressed())
        {
            if (!context->debugOverlay->IsShown())
            {
                context->debugOverlay->Show();
            }
            else
            {
                context->debugOverlay->Hide();
            }
        }

        keyState = kb->GetKeyState(DAVA::eInputElements::KB_ESCAPE);
        if (keyState.IsJustPressed())
        {
            eCursorCapture c = GetPrimaryWindow()->GetCursorCapture();
            if (c == eCursorCapture::PINNING)
            {
                GetPrimaryWindow()->SetCursorCapture(eCursorCapture::OFF);
            }
            else
            {
                GetPrimaryWindow()->SetCursorCapture(eCursorCapture::PINNING);
            }
        }
    }
}
