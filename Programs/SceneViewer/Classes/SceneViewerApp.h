#pragma once

#include "Settings.h"

#ifdef WITH_SCENE_PERFORMANCE_TESTS
#include <GridTest.h>
#endif

#include <Network/ServicesProvider.h>
#include <DAVAEngine.h>
#include <Base/ScopedPtr.h>
#include <Scene3D/Scene.h>
#include <UI/UI3DView.h>
#include <FileSystem/FilePath.h>

namespace DAVA
{
class Engine;
class Window;
}

class ViewSceneScreen;
class PerformanceResultsScreen;

struct SceneViewerData
{
    DAVA::Engine& engine;
    Settings settings;
    DAVA::float32 screenAspect;
    DAVA::FilePath scenePath;
    DAVA::ScopedPtr<DAVA::Scene> scene;
#ifdef WITH_SCENE_PERFORMANCE_TESTS
    GridTestResult gridTestResult;
#endif
};

class SceneViewerApp final
{
public:
    SceneViewerApp(DAVA::Engine& e);

    void OnAppStarted();
    void OnWindowCreated(DAVA::Window* w);
    void OnAppFinished();

    void OnSuspend();
    void OnResume();

    void BeginFrame();
    void Draw(DAVA::Window* window);
    void EndFrame();

private:
    void CreateDocumentsFolder();

    ViewSceneScreen* viewSceneScreen = nullptr;
    PerformanceResultsScreen* performanceResultsScreen = nullptr;

    SceneViewerData data;

    std::shared_ptr<DAVA::Net::NetService> netLogger;
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    std::shared_ptr<DAVA::Net::NetService> memprofServer;
#endif
    std::unique_ptr<DAVA::Net::ServicesProvider> servicesProvider;
};
