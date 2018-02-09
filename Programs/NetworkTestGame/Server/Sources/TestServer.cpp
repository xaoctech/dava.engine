#include "TestServer.h"

#include "Game.h"
#include "ShooterConstants.h"
#include "ShooterUtils.h"
#include "Systems/GameInputSystem.h"
#include "Systems/PlayerEntitySystem.h"
#include "Systems/BotSystem.h"
#include "Systems/MarkerSystem.h"
#include "Systems/GameModeSystem.h"
#include "Systems/GameModeSystemCars.h"
#include "Systems/GameModeSystemCharacters.h"
#include <Systems/GameVisibilitySystem.h>

#include <CommandLine/CommandLineParser.h>
#include <Concurrency/Thread.h>
#include <Debug/DVAssertDefaultHandlers.h>
#include <Debug/ProfilerUtils.h>
#include <Debug/ProfilerCPU.h>
#include <Debug/ProfilerGPU.h>
#include <Debug/ProfilerOverlay.h>
#include <Engine/Engine.h>
#include <Entity/Component.h>
#include <Entity/ComponentManager.h>
#include <Logger/Logger.h>
#include <MemoryManager/MemoryLogger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Renderer.h>
#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkStatisticsSingleComponent.h>
#include <NetworkCore/Scene3D/Systems/NetworkGameModeSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkIdSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkInputSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkReplicationSystem2.h>
#include <NetworkCore/Scene3D/Systems/NetworkTimeSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkTimelineControlSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkVisibilitySystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemServer.h>
#include <NetworkCore/Scene3D/Systems/SnapshotSystemServer.h>
#include <NetworkCore/Scene3D/Systems/NetworkRemoteInputSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkHealthCheckSystem.h>
#include <Scene3D/Systems/TransformSystem.h>
#include <Scene3D/Systems/DiffMonitoringSystem.h>
#include <Scene3D/Systems/Controller/WASDControllerSystem.h>
#include <Scene3D/Systems/Controller/RotationControllerSystem.h>
#include <UI/UIControlSystem.h>

#include <Physics/PhysicsSystem.h>

#include <NetworkPhysics/NetworkPhysicsSystem.h>

//////////////////
#include <NetworkCore/Scene3D/Systems/NetworkTransformFromLocalToNetSystem.h>
#include "Components/SingleComponents/GameModeSingleComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"

#if defined(__DAVAENGINE_LINUX__) || defined(__DAVAENGINE_MACOS__)
#include <signal.h>
#endif

#if defined(__CRASHHANDLER_ENABLED__)
#include <CrashHandler/CrashHandlerModule.h>
#endif

using namespace DAVA;

static const uint32 HOST = 0; // Any interface

int DAVAMain(Vector<String> cmdline)
{
#if defined(__CRASHHANDLER_ENABLED__)
    InstallCrashHandler("/debug/server");
#endif
    Assert::SetupDefaultHandlers();

    Engine e;

    bool isGUI = CommandLineParser::CommandIsFound("--gui");
    String gameName = CommandLineParser::GetCommandParam("--game");
    GameMode::Id gameModeId = GameMode::IdByName(gameName);

    KeyedArchive* appOptions = new KeyedArchive();
    eEngineRunMode runmode;
    Vector<String> modules =
    {
      "JobManager",
      "SoundSystem",
      "LocalizationSystem"
    };
    if (isGUI)
    {
        runmode = eEngineRunMode::GUI_STANDALONE;
        appOptions->SetInt32("rhi_threaded_frame_count", 2);
    }
    else
    {
#if defined(__DAVAENGINE_LINUX__) || defined(__DAVAENGINE_MACOS__)
        signal(SIGINT, [](int) {
            Engine::Instance()->QuitAsync(0);
        });
#endif
        runmode = eEngineRunMode::CONSOLE_MODE;
        appOptions->SetInt32("renderer", rhi::RHI_NULL_RENDERER);
    }

    RegisterGameComponents();

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
    e.Init(runmode, modules, appOptions);

    String portStr = CommandLineParser::GetCommandParam("--port");
    uint16 port = 9000;
    if (!portStr.empty())
    {
        port = std::stoi(portStr);
    }

    String clientsStr = CommandLineParser::GetCommandParam("--clients");
    uint8 clientsNumber = 100;
    if (!clientsStr.empty())
    {
        clientsNumber = std::stoi(clientsStr);
    }

    TestServer app(e, gameModeId, port, clientsNumber);
    return e.Run();
}

TestServer::TestServer(Engine& engine, GameMode::Id gameModeId, uint16 port, uint8 clientsNumber)
    : engine(engine)
    , gameModeId(gameModeId)
    , gameServer(HOST, port, clientsNumber)
{
    profilePath = CommandLineParser::GetCommandParam("--profile");
    isGUI = CommandLineParser::CommandIsFound("--gui");
    hasPoisonPill = CommandLineParser::CommandIsFound("--poisonpill");
    String botsStr = CommandLineParser::GetCommandParam("--bots");
    gameStatsLogPath = CommandLineParser::GetCommandParam("--game-stats-log");
    enableVisibilityLods = CommandLineParser::CommandIsFound("--visibility");
    if (!botsStr.empty())
    {
        botsCount = std::stoi(botsStr);
    }

    String lossFactorStr = CommandLineParser::GetCommandParam("--loss-factor");
    if (!lossFactorStr.empty())
    {
        lossFactor = std::stof(lossFactorStr);
    }

    String freqHzStr = CommandLineParser::GetCommandParam("--hz");
    if (!freqHzStr.empty())
    {
        freqHz = std::stoi(freqHzStr);
    }

    healthCheckHost = CommandLineParser::GetCommandParam("--health-check-host");
    if (!healthCheckHost.empty())
    {
        String healthCheckPortStr = CommandLineParser::GetCommandParam("--health-check-port");
        if (!healthCheckPortStr.empty())
        {
            healthCheckPort = std::stoi(healthCheckPortStr);
        }
    }

    {
        String type = CommandLineParser::GetCommandParam("--synthetic-crash");
        if (type == "null_pointer")
            syntheticCrashType = CrashType::NULL_POINTER;
        else if (type == "memory_corruption")
            syntheticCrashType = CrashType::MEMORY_CORRUPTION;
        else if (type == "unhandled_exception")
            syntheticCrashType = CrashType::UNHANDLED_EXCEPTION;
        else if (type == "double_free")
            syntheticCrashType = CrashType::DOUBLE_FREE;

        String site = CommandLineParser::GetCommandParam("--synthetic-crash-site");
        if (site == "main_thread")
            syntheticCrashSite = CrashSite::MAIN_THREAD;
        else if (site == "job_manager")
            syntheticCrashSite = CrashSite::JOB_MANAGER;
        else if (site == "other_thread")
            syntheticCrashSite = CrashSite::OTHER_THREAD;
    }

    if (isGUI)
    {
        engine.windowCreated.Connect(this, &TestServer::OnWindowCreated);
    }
    else
    {
        GetEngineContext()->uiControlSystem->vcs->RegisterAvailableResourceSize(1024, 768, "Gfx");
    }

    engine.gameLoopStarted.Connect(this, &TestServer::OnLoopStarted);
    engine.gameLoopStopped.Connect(this, &TestServer::OnLoopStopped);
    engine.cleanup.Connect(this, &TestServer::OnEngineCleanup);

    engine.suspended.Connect(this, &TestServer::OnSuspended);
    engine.resumed.Connect(this, &TestServer::OnResumed);

    engine.update.Connect(this, &TestServer::OnUpdate);
}

void TestServer::OnWindowCreated(DAVA::Window* w)
{
    VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;

    const Size2i& physicalSize = vcs->GetPhysicalScreenSize();
    float32 screenAspect = static_cast<float32>(physicalSize.dx) / static_cast<float32>(physicalSize.dy);

    const Size2f windowSize = { 1024.f, 1024.f / screenAspect };
    String title = Format("DAVA Engine - TestServer | [%u bit]",
                          static_cast<uint32>(sizeof(pointer_size) * 8));
    w->SetTitleAsync(title);
    w->SetSizeAsync(windowSize);
    w->SetVirtualSize(windowSize.dx, windowSize.dy);

    vcs->RegisterAvailableResourceSize(static_cast<int32>(windowSize.dx),
                                       static_cast<int32>(windowSize.dy),
                                       "Gfx");

    CreateScene(screenAspect);

    mainScreen = new MainScreen(scene);
    UIScreenManager::Instance()->RegisterScreen(mainScreen->GetScreenID(), mainScreen);
    UIScreenManager::Instance()->SetFirst(mainScreen->GetScreenID());
}

void TestServer::OnLoopStarted()
{
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    memoryLogger = std::make_unique<MemoryLogger>();
#endif

    if (!profilePath.empty())
    {
        ProfilerCPU::globalProfiler->Start();
        ProfilerGPU::globalProfiler->Start();
        if (isGUI)
        {
            ProfilerOverlay::globalProfilerOverlay->SetInputEnabled(true);
        }
    }

    Logger::Debug("****** TestServer::OnGameLoopStarted");

    if (!isGUI)
    {
        CreateScene(1.f);
    }
}

void TestServer::OnLoopStopped()
{
    Logger::Debug("****** TestServer::OnGameLoopStopped");

#ifndef USE_SNAPSHOT_SYSTEM
    scene->RemoveSystem(networkTransportSystem);
    SafeDelete(networkTransportSystem);
#endif

    SafeRelease(mainScreen);
    scene.reset();
}

void TestServer::OnEngineCleanup()
{
    Logger::Debug("****** TestServer::OnEngineCleanup");
}

void TestServer::OnSuspended()
{
    Logger::Debug("****** TestBed::OnSuspended");
}

void TestServer::OnResumed()
{
    Logger::Debug("****** TestBed::OnResumed");
}

void TestServer::GenerateCrash(CrashType type)
{
    switch (type)
    {
    case CrashType::NULL_POINTER:
    {
        Logger::Info("Synthetic crash: null pointer access");
        volatile int* p = nullptr;
        *p = 0xDEAD0000;
        break;
    }
    case CrashType::MEMORY_CORRUPTION:
    {
        Logger::Info("Synthetic crash: memory corruption");
        char* memory = new char[4];
        memcpy(memory - 64, static_cast<void*>(Engine::Instance()), 128);
        delete[] memory;
        break;
    }
    case CrashType::UNHANDLED_EXCEPTION:
        Logger::Info("Synthetic crash: unhandled C++ exception");
        throw std::runtime_error("Kaboom! Unhandled C++ exception");
        break;
    case CrashType::DOUBLE_FREE:
    {
        Logger::Info("Synthetic crash: double free memory block");
        volatile int* p = new int(10);
        *p = 42;
        delete p;
        delete p;
        break;
    }
    default:
        break;
    }
}

void TestServer::OnUpdate(float32 frameDelta)
{
    DAVA_PROFILER_CPU_SCOPE("TestServer::OnUpdate");
    gameServer.Update(frameDelta);

    if (mainScreen)
    {
        mainScreen->Update(frameDelta);
    }
    else if (scene)
    {
        scene->Update(frameDelta);
    }

    syntheticCrashCountDown -= 1;
    if (syntheticCrashCountDown == 0)
    {
        auto doCrash = [](CrashSite site, CrashType type) {
            switch (site)
            {
            case CrashSite::MAIN_THREAD:
                GenerateCrash(type);
                break;
            case CrashSite::JOB_MANAGER:
                GetEngineContext()->jobManager->CreateWorkerJob([type]() { GenerateCrash(type); });
                break;
            case CrashSite::OTHER_THREAD:
            {
                Thread* t = Thread::Create([type]() { GenerateCrash(type); });
                t->Start();
                t->Release();
                break;
            }
            }

        };
        doCrash(syntheticCrashSite, syntheticCrashType);
    }
}

void TestServer::AddTokenToGame(NetworkGameModeSingleComponent* netGameModeComp, const FastName& token) const
{
    netGameModeComp->AddValidToken(token);
    NetworkPlayerID playerID = netGameModeComp->GetNextNetworkPlayerID();
    netGameModeComp->AddNetworkPlayerID(token, playerID);
    Logger::Debug("Token: %s, Player ID: %d", token.c_str(), playerID);
}

void TestServer::CreateScene(DAVA::float32 screenAspect)
{
    tags.insert({ FastName("server"), FastName("input"), FastName("network"), FastName("marker") });

    if (botsCount > 0)
    {
        tags.insert({ FastName("bot"), FastName("randombot") });
    }

    switch (gameModeId)
    {
    case GameMode::Id::HELLO:
        // broken
        break;
    case GameMode::Id::CHARACTERS:
        tags.insert(FastName("gm_characters"));
        break;
    case GameMode::Id::PHYSICS:
        // broken
        break;
    case GameMode::Id::CARS:
        tags.insert(FastName("gm_cars"));
        break;
    case GameMode::Id::SHOOTER:
        tags.insert({ FastName("gm_shooter"), FastName("controller") });
        break;
    default:
        tags.insert({ FastName("gm_tanks"), FastName("shoot") });
        break;
    }

    bool isShooterGm = tags.find(FastName("gm_shooter")) != tags.end();

    if (!isShooterGm)
    {
        tags.insert({ FastName("gameinput"), FastName("playerentity") });
    }

    if (!gameStatsLogPath.empty())
    {
        tags.insert(FastName("log_game_stats"));
    }

    scene = new Scene(tags);
    scene->SetPerformFixedProcessOnlyOnce(true);

    ScopedPtr<Camera> camera(new Camera());
    scene->AddCamera(camera);
    scene->SetCurrentCamera(camera);

    NetworkGameModeSingleComponent* netGameModeComp = scene->GetSingletonComponent<NetworkGameModeSingleComponent>();
    netGameModeComp->SetGameModeType(GameModeSystem::WAITING);
    netGameModeComp->SetMapName(FastName("map_01.sc2"));

    BattleOptionsSingleComponent* optionsSingleComponent = scene->GetSingletonComponent<BattleOptionsSingleComponent>();
    optionsSingleComponent->gameModeId = gameModeId;
    optionsSingleComponent->isEnemyPredicted = CommandLineParser::CommandIsFound("--predict_enemy");
    optionsSingleComponent->gameStatsLogPath = gameStatsLogPath;
    optionsSingleComponent->compareInputs = CommandLineParser::CommandIsFound("--compare_inputs");
    optionsSingleComponent->isSet = true;

    NetworkTimeSingleComponent::SetFrequencyHz(static_cast<float32>(freqHz));

    scene->GetSingletonComponent<NetworkServerSingleComponent>()->SetServer(&gameServer.GetUDPServer());

    scene->CreateSystemsByTags();

    if (scene->GetSystem<PhysicsSystem>() != nullptr)
    {
        scene->GetSystem<PhysicsSystem>()->SetSimulationEnabled(true);
    }

    if (!healthCheckHost.empty())
    {
        NetworkHealthCheckSystem* netHealthCheckSystem = scene->GetSystem<NetworkHealthCheckSystem>();
        netHealthCheckSystem->Connect(healthCheckHost, healthCheckPort);
    }

    NetworkRemoteInputSystem* remoteInputSystem = scene->GetSystem<NetworkRemoteInputSystem>();
    remoteInputSystem->SetFullInputComparisonFlag(optionsSingleComponent->compareInputs);

    NetworkTimeSystem* networkTimeSystem = scene->GetSystem<NetworkTimeSystem>();
    networkTimeSystem->SetLossFactor(lossFactor);

    if (isShooterGm)
    {
        MarkerSystem* markerSystem = scene->GetSystem<MarkerSystem>();
        markerSystem->SetHealthParams(SHOOTER_CHARACTER_MAX_HEALTH, 0.02f);

        scene->GetSystem<WASDControllerSystem>()->SetMoveSpeed(50.0f);

        SetupShooterServerCamera();

        InitializeScene(*scene);
    }

    if (enableVisibilityLods)
    {
        scene->GetSystem<GameVisibilitySystem>()->SetMaxAOI(500.f);
        scene->GetSystem<GameVisibilitySystem>()->SetPeriodIncreaseDistance(63.f);
    }

    gameServer.Setup(GameServer::Options{
    scene,
    scene->GetSystem<NetworkInputSystem>(),
    isGUI,
    profilePath,
    hasPoisonPill,
    });

#ifdef NDEBUG
    for (int32 host = 1; host <= 2; ++host)
    {
        for (int32 i = 1; i <= 60; ++i)
        {
            int32 clientID = i * 10 + host;
            FastName token(Format("%064d", clientID));
            AddTokenToGame(netGameModeComp, token);
        }
    }
#endif

    if (botsCount > 0)
    {
        scene->GetSystem<BotSystem>()->SetBotsCount(botsCount);

        auto IsGm = [this](const char* tag) { return tags.find(FastName(tag)) != tags.end(); };

        if (IsGm("gm_tanks"))
            AddServerBots(netGameModeComp, scene->GetSystem<GameModeSystem>());
        if (IsGm("gm_cars"))
            AddServerBots(netGameModeComp, scene->GetSystem<GameModeSystemCars>());
        if (IsGm("gm_characters"))
            AddServerBots(netGameModeComp, scene->GetSystem<GameModeSystemCharacters>());
        if (IsGm("gm_shooter"))
            DVASSERT(false, "No bots for shooter mode ftm.");
    }
}

void TestServer::SetupShooterServerCamera()
{
    // Server-only camera

    VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
    Size2i physicalWindowSize = vcs->GetPhysicalScreenSize();
    float32 screenAspectRatio = static_cast<float32>(physicalWindowSize.dx) / static_cast<float32>(physicalWindowSize.dy);

    ScopedPtr<Camera> camera(new Camera());
    camera->SetUp(Vector3(0.f, 0.f, 1.f));
    camera->SetPosition(Vector3(0.f, 20.0f, 50.0f));
    camera->SetTarget(Vector3(0.f, 0.f, 0.f));
    camera->RebuildCameraFromValues();
    camera->SetupPerspective(70.f, screenAspectRatio, 0.5f, 2500.f);
    scene->AddCamera(camera);
    scene->SetCurrentCamera(camera);

    CameraComponent* cameraComponent = new CameraComponent();
    cameraComponent->SetCamera(camera);

    Entity* cameraEntity = new Entity();
    cameraEntity->SetName("ServerOnlyCamera");
    cameraEntity->AddComponent(cameraComponent);
    cameraEntity->AddComponent(new WASDControllerComponent());
    cameraEntity->AddComponent(new RotationControllerComponent());

    scene->AddNode(cameraEntity);
}

template <typename T>
void TestServer::AddServerBots(NetworkGameModeSingleComponent* netGameModeComp, T* gameModeSystem)
{
    BotSystem* botSystem = scene->GetSystem<BotSystem>();

    if (botSystem == nullptr)
    {
        return;
    }

    const Vector<BotResponder>& botResponders = botSystem->GetBotResponders();
    for (const BotResponder& botResponder : botResponders)
    {
#ifndef NDEBUG
        AddTokenToGame(netGameModeComp, botResponder.GetToken());
#endif
        gameModeSystem->OnClientConnected(botResponder);
    }
}
