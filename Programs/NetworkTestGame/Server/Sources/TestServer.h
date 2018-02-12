#pragma once

#include "Game.h"
#include "GameServer.h"
#include "UIScreen/MainScreen.h"

#include <Base/BaseTypes.h>
#include <Base/ScopedPtr.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
class Engine;
class MemoryLogger;
class Window;
class NetworkTimeSystem;
class NetworkGameModeSingleComponent;
}

class BotSystem;

class TestServer
{
public:
    TestServer(DAVA::Engine& engine, GameMode::Id gameModeId, DAVA::uint16 port, DAVA::uint8 clientsNumber);

    void OnWindowCreated(DAVA::Window* w);

    void OnLoopStarted();
    void OnLoopStopped();
    void OnEngineCleanup();

    void OnSuspended();
    void OnResumed();

    void OnUpdate(DAVA::float32 frameDelta);

private:
    DAVA::Engine& engine;
    DAVA::ScopedPtr<DAVA::Scene> scene;
    DAVA::UnorderedSet<DAVA::FastName> tags;
    MainScreen* mainScreen = nullptr;

    GameMode::Id gameModeId;
    GameServer gameServer;

    DAVA::String profilePath;
    bool isGUI = false;
    bool hasPoisonPill = false;
    DAVA::int32 botsCount = 0;
    DAVA::String gameStatsLogPath;
    bool enableVisibilityLods = false;
    DAVA::float32 lossFactor = 0.05f; // every 5% increase bucket size
    DAVA::uint32 freqHz = 60;
    DAVA::String healthCheckHost;
    DAVA::uint16 healthCheckPort = 5050;

    void AddTokenToGame(DAVA::NetworkGameModeSingleComponent* netGameModeComp, const DAVA::FastName& token) const;
    void CreateScene(DAVA::float32 screenAspect);

    void SetupShooterServerCamera();

    template <typename T>
    void AddServerBots(DAVA::NetworkGameModeSingleComponent* netGameModeComp, T* gameModeSystem);

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    std::unique_ptr<DAVA::MemoryLogger> memoryLogger;
#endif

    enum class CrashType
    {
        NOCRASH = 0,
        NULL_POINTER,
        MEMORY_CORRUPTION,
        UNHANDLED_EXCEPTION,
        DOUBLE_FREE,
    };
    enum class CrashSite
    {
        MAIN_THREAD = 0,
        JOB_MANAGER,
        OTHER_THREAD,
    };
    CrashType syntheticCrashType = CrashType::NOCRASH;
    CrashSite syntheticCrashSite = CrashSite::MAIN_THREAD;
    int syntheticCrashCountDown = 600;

    static void GenerateCrash(CrashType type);
};
