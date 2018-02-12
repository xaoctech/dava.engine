#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>

namespace DAVA
{
class Engine;
class MemoryLogger;
class Window;
class KeyedArchive;
}

class Battle;

class TestClient
{
public:
    TestClient(DAVA::Engine& engine, DAVA::KeyedArchive* clientOptions);

    void OnLoopStarted();

    void OnWindowCreated(DAVA::Window* w);
    void Draw(DAVA::Window* w);

    void OnLoopStopped();
    void OnEngineCleanup();

    void OnSuspended();
    void OnResumed();

    void OnUpdate(DAVA::float32 frameDelta);
    void OnWindowUpdate(DAVA::Window* window, DAVA::float32 frameDelta);

    static DAVA::KeyedArchive* CreateOptions();

private:
    void DebugInit();
    void DebugUpdate();

    DAVA::Engine& engine;
    DAVA::KeyedArchive* clientOptions = nullptr;
    std::unique_ptr<Battle> botBattle;

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    std::unique_ptr<DAVA::MemoryLogger> memoryLogger;
#endif
};
