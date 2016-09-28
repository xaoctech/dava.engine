#include "Infrastructure/TestBed.h"

#include "Engine/EngineModule.h"

#include "Platform/DateTime.h"
#include "CommandLine/CommandLineParser.h"
#include "Utils/Utils.h"
#include "Infrastructure/TestListScreen.h"
#include "Tests/NotificationTest.h"
#include "Tests/UIScrollViewTest.h"
#include "Tests/SpeedLoadImagesTest.h"
#include "Tests/MultilineTest.h"
#include "Tests/StaticTextTest.h"
#include "Tests/StaticWebViewTest.h"
#include "Tests/MicroWebBrowserTest.h"
#include "Tests/UIMovieTest.h"
#include "Tests/FontTest.h"
#include "Tests/WebViewTest.h"
#include "Tests/FunctionSignalTest.h"
#include "Tests/KeyboardTest.h"
#include "Tests/FullscreenTest.h"
#include "Tests/UIBackgroundTest.h"
#include "Tests/ClipTest.h"
#include "Tests/FloatingPointExceptionTest.h"
#include "Tests/DlcTest.h"
#include "Tests/FormatsTest.h"
#include "Tests/GPUTest.h"
#include "Tests/PackManagerTest.h"
#include "Tests/AssertTest.h"
#include "Tests/CoreV2Test.h"
#include "Tests/DeviceInfoTest.h"
//$UNITTEST_INCLUDE

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
#include "MemoryManager/MemoryProfiler.h"
#endif

void CheckDeviceInfoValid();

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
    using namespace DAVA;
    using namespace Net;

    KeyedArchive* appOptions = new KeyedArchive();
    appOptions->SetString("title", "TestBed");
    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetInt32("rhi_threaded_frame_count", 2);
    appOptions->SetInt32("width", 1024);
    appOptions->SetInt32("height", 768);
#if defined(__DAVAENGINE_QT__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#elif defined(__DAVAENGINE_MACOS__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#elif defined(__DAVAENGINE_IPHONE__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#elif defined(__DAVAENGINE_WIN32__)
    appOptions->SetInt32("renderer", rhi::RHI_DX9);
#elif defined(__DAVAENGINE_WIN_UAP__)
    appOptions->SetInt32("renderer", rhi::RHI_DX11);
#elif defined(__DAVAENGINE_ANDROID__)
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
#endif

    eEngineRunMode runmode = eEngineRunMode::GUI_STANDALONE;
    if (cmdline.size() > 1 && cmdline[1] == "--console")
    {
        runmode = eEngineRunMode::CONSOLE_MODE;
    }

    Vector<String> modules = {
        "JobManager",
        "NetCore",
        "LocalizationSystem",
        "SoundSystem",
        "DownloadManager",
        "PackManager"
    };

    Engine e;
    e.SetOptions(appOptions);
    e.Init(runmode, modules);

    CheckDeviceInfoValid();

    TestBed game(e);
    return e.Run();
}

TestBed::TestBed(Engine& engine)
    : engine(engine)
    , currentScreen(nullptr)
    , testListScreen(nullptr)
{
    engine.gameLoopStarted.Connect(this, &TestBed::OnGameLoopStarted);
    engine.gameLoopStopped.Connect(this, &TestBed::OnGameLoopStopped);
    engine.cleanup.Connect(this, &TestBed::OnEngineCleanup);

    engine.suspended.Connect(this, &TestBed::OnSuspended);
    engine.resumed.Connect(this, &TestBed::OnResumed);

    if (engine.IsConsoleMode())
    {
        engine.update.Connect(this, &TestBed::OnUpdateConsole);
    }
    else
    {
        engine.windowCreated.Connect(this, &TestBed::OnWindowCreated);
        engine.windowDestroyed.Connect(this, &TestBed::OnWindowDestroyed);

        Window* w = engine.PrimaryWindow();
        w->SetTitle("[Testbed] The one who owns a minigun fears not");
        w->Resize(1024.f, 768.f);
        w->sizeScaleChanged.Connect(this, &TestBed::OnWindowSizeChanged);
    }

    engine.GetContext()->uiControlSystem->SetClearColor(Color::Black);
}

void TestBed::OnGameLoopStarted()
{
    Logger::Debug("****** TestBed::OnGameLoopStarted");

    UIYamlLoader::LoadFonts("~res:/UI/Fonts/fonts.yaml");

    InitNetwork();
    RunOnlyThisTest();

    if (engine.IsConsoleMode())
    {
        engine.RunAsyncOnMainThread([]() {
            Logger::Error("******** KABOOM on main thread********");
        });
    }
}

void TestBed::OnGameLoopStopped()
{
    Logger::Debug("****** TestBed::OnGameLoopStopped");

    for (auto testScreen : screens)
    {
        SafeRelease(testScreen);
    }
    screens.clear();
    SafeRelease(testListScreen);
}

void TestBed::OnEngineCleanup()
{
    Logger::Debug("****** TestBed::OnEngineCleanup");
    netLogger.Uninstall();
}

void TestBed::OnWindowCreated(DAVA::Window* w)
{
    Logger::Error("****** TestBed::OnWindowCreated");

    testListScreen = new TestListScreen();
    UIScreenManager::Instance()->RegisterScreen(0, testListScreen);
    RegisterTests();
    RunTests();
}

void TestBed::OnWindowDestroyed(DAVA::Window* w)
{
    Logger::Error("****** TestBed::OnWindowDestroyed");
}

void TestBed::OnWindowSizeChanged(DAVA::Window* w, DAVA::float32 width, DAVA::float32 height, DAVA::float32 scaleX, DAVA::float32 scaleY)
{
    Logger::Debug("********** TestBed::OnWindowSizeChanged: w=%.1f, h=%.1f, sx=%.1f, sy=%.1f", width, height, scaleX, scaleY);
}

void TestBed::OnSuspended()
{
    Logger::Error("****** TestBed::OnSuspended");
}

void TestBed::OnResumed()
{
    Logger::Error("****** TestBed::OnResumed");
}

void TestBed::OnUpdateConsole(DAVA::float32 frameDelta)
{
    static int frameCount = 0;
    frameCount += 1;
    Logger::Debug("****** update: count=%d, delta=%f", frameCount, frameDelta);
    if (frameCount >= 100)
    {
        Logger::Debug("****** quit");
        engine.Quit();
    }
}

void TestBed::RunOnlyThisTest()
{
    //runOnlyThisTest = "NotificationScreen";
}

void TestBed::OnError()
{
    DavaDebugBreak();
}

void TestBed::RegisterTests()
{
    new CoreV2Test(*this);
    new DeviceInfoTest(*this);
    new DlcTest(*this);
    new UIScrollViewTest(*this);
    new NotificationScreen(*this);
    new SpeedLoadImagesTest(*this);
    new MultilineTest(*this);
    new StaticTextTest(*this);
    new StaticWebViewTest(*this);
    new MicroWebBrowserTest(*this);
    new UIMovieTest(*this);
    new FontTest(*this);
    new WebViewTest(*this);
    new FunctionSignalTest(*this);
    new KeyboardTest(*this);
    new FullscreenTest(*this);
    new UIBackgroundTest(*this);
    new ClipTest(*this);
    new GPUTest(*this);
    new FormatsTest(*this);
    new AssertTest(*this);
    new FloatingPointExceptionTest(*this);
    new PackManagerTest(*this);
    //$UNITTEST_CTOR
}

void TestBed::RegisterScreen(BaseScreen* screen)
{
    UIScreenManager::Instance()->RegisterScreen(screen->GetScreenId(), screen);

    screens.push_back(screen);
    testListScreen->AddTestScreen(screen);
}

void TestBed::ShowStartScreen()
{
    currentScreen = nullptr;
    UIScreenManager::Instance()->SetScreen(0);
}

void TestBed::CreateDocumentsFolder()
{
    FilePath documentsPath = FileSystem::Instance()->GetUserDocumentsPath() + "TestBed/";

    FileSystem::Instance()->CreateDirectory(documentsPath, true);
    FileSystem::Instance()->SetCurrentDocumentsDirectory(documentsPath);
}

File* TestBed::CreateDocumentsFile(const String& filePathname)
{
    FilePath workingFilepathname = FilePath::FilepathInDocuments(filePathname);

    FileSystem::Instance()->CreateDirectory(workingFilepathname.GetDirectory(), true);

    File* retFile = File::Create(workingFilepathname, File::CREATE | File::WRITE);
    return retFile;
}

void TestBed::RunTests()
{
    if ("" != runOnlyThisTest)
    {
        for (auto screen : screens)
        {
            if (!IsNeedSkipTest(*screen))
            {
                currentScreen = screen;
            }
        }
    }
    else
    {
        currentScreen = nullptr;
    }

    if (nullptr != currentScreen)
    {
        UIScreenManager::Instance()->SetScreen(currentScreen->GetScreenId());
    }
    else
    {
        UIScreenManager::Instance()->SetScreen(0);
    }
}

bool TestBed::IsNeedSkipTest(const BaseScreen& screen) const
{
    if (runOnlyThisTest.empty())
    {
        return false;
    }

    const FastName& name = screen.GetName();

    return 0 != CompareCaseInsensitive(runOnlyThisTest, name.c_str());
}

void TestBed::InitNetwork()
{
    using namespace Net;
    auto loggerCreate = [this](uint32 serviceId, void*) -> IChannelListener* {
        if (!loggerInUse)
        {
            loggerInUse = true;
            return &netLogger;
        }
        return nullptr;
    };
    NetCore::Instance()->RegisterService(NetCore::SERVICE_LOG, loggerCreate,
                                         [this](IChannelListener* obj, void*) -> void { loggerInUse = false; });

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    auto memprofCreate = [this](uint32 serviceId, void*) -> IChannelListener* {
        if (!memprofInUse)
        {
            memprofInUse = true;
            return &memprofServer;
        }
        return nullptr;
    };
    NetCore::Instance()->RegisterService(NetCore::SERVICE_MEMPROF, memprofCreate,
                                         [this](IChannelListener* obj, void*) -> void { memprofInUse = false; });
#endif

    eNetworkRole role = SERVER_ROLE;
    Net::Endpoint endpoint = Net::Endpoint(NetCore::DEFAULT_TCP_PORT);

    NetConfig config(role);
    config.AddTransport(TRANSPORT_TCP, endpoint);
    config.AddService(NetCore::SERVICE_LOG);
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    config.AddService(NetCore::SERVICE_MEMPROF);
#endif
    peerDescr = PeerDescription(config);
    Net::Endpoint annoUdpEndpoint(NetCore::defaultAnnounceMulticastGroup, NetCore::DEFAULT_UDP_ANNOUNCE_PORT);
    Net::Endpoint annoTcpEndpoint(NetCore::DEFAULT_TCP_ANNOUNCE_PORT);
    id_anno = NetCore::Instance()->CreateAnnouncer(annoUdpEndpoint, DEFAULT_ANNOUNCE_TIME_PERIOD, MakeFunction(this, &TestBed::AnnounceDataSupplier), annoTcpEndpoint);
    id_net = NetCore::Instance()->CreateController(config, nullptr);
}

size_t TestBed::AnnounceDataSupplier(size_t length, void* buffer)
{
    using namespace Net;
    if (true == peerDescr.NetworkInterfaces().empty())
    {
        peerDescr.SetNetworkInterfaces(NetCore::Instance()->InstalledInterfaces());
    }
    return peerDescr.Serialize(buffer, length);
}

void CheckDeviceInfoValid()
{
    using namespace DAVA;
    Logger::Info("device info begin==========================================");

    auto platform = DeviceInfo::GetPlatform();
    Logger::Info("platform enum index: %d", platform);
    DVASSERT(DeviceInfo::PLATFORM_UNKNOWN_VALUE != platform);

    auto platformString = DeviceInfo::GetPlatformString();
    Logger::Info("platform name: %s", platformString.c_str());
    DVASSERT(platformString != "Unknown");

    auto version = DeviceInfo::GetVersion();
    Logger::Info("version: %s", version.c_str());
    DVASSERT(version != "");

    auto manufacturer = DeviceInfo::GetManufacturer();
    Logger::Info("manufacturer: %s", manufacturer.c_str());

    auto model = DeviceInfo::GetModel();
    Logger::Info("model: %s", model.c_str());

    auto locale = DeviceInfo::GetLocale();
    Logger::Info("locale: %s", locale.c_str());
    DVASSERT(locale != "");

    auto region = DeviceInfo::GetRegion();
    Logger::Info("region: %s", region.c_str());
    DVASSERT(region != "");

    auto timezone = DeviceInfo::GetTimeZone();
    Logger::Info("timezone: %s", timezone.c_str());
    DVASSERT(timezone != "");

    auto udid = DeviceInfo::GetUDID();
    Logger::Info("udid: %s", udid.c_str());
    DVASSERT(udid != "");

    auto name = DeviceInfo::GetName();
    Logger::Info("name: %s", UTF8Utils::EncodeToUTF8(name).c_str());
    DVASSERT(name != L"");

    auto httpProxyHost = DeviceInfo::GetHTTPProxyHost();
    Logger::Info("http_proxy_host: %s", httpProxyHost.c_str());
    DVASSERT(httpProxyHost == "");

    auto httpNonProxyHosts = DeviceInfo::GetHTTPNonProxyHosts();
    Logger::Info("http_non_proxy_host: %s", httpNonProxyHosts.c_str());
    DVASSERT(httpNonProxyHosts == "");

    auto httpProxyPort = DeviceInfo::GetHTTPProxyPort();
    Logger::Info("http_proxy_port: %d", httpProxyPort);
    DVASSERT(httpProxyPort == 0);

    auto screenInfo = DeviceInfo::GetScreenInfo();
    Logger::Info("screen_info: w=%d h=%d scale=%f", screenInfo.width, screenInfo.height, screenInfo.scale);
    DVASSERT(screenInfo.height > 0);
    DVASSERT(screenInfo.width > 0);
    DVASSERT(screenInfo.scale >= 1);

    auto zbufferSize = DeviceInfo::GetZBufferSize();
    Logger::Info("zbuffer_size: %d", zbufferSize);
    DVASSERT(zbufferSize == 16 || zbufferSize == 24);

    auto gpuFamily = DeviceInfo::GetGPUFamily();
    Logger::Info("gpu_family enum index: %d", gpuFamily);
    DVASSERT(gpuFamily != GPU_INVALID);

    auto networkInfo = DeviceInfo::GetNetworkInfo();
    Logger::Info("network: type=%d signal_strength=%d", networkInfo.networkType, networkInfo.signalStrength);

    List<DeviceInfo::StorageInfo> storageInfo = DeviceInfo::GetStoragesList();
    for (const auto& info : storageInfo)
    {
        Logger::Info("storage info: type=%d total_space=%lld free_space=%lld, read_only=%d, removable=%d, emulated=%d",
                     info.type, info.totalSpace, info.freeSpace, info.readOnly, info.removable, info.emulated);
    }

    uint32 cpuCount = DeviceInfo::GetCpuCount();
    Logger::Info("cpu_count: %d", cpuCount);
    DVASSERT(cpuCount > 0);
    Logger::Info("device info end============================================");
}
