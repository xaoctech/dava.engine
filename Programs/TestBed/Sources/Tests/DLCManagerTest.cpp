#include "Tests/DLCManagerTest.h"
#include "Infrastructure/TestBed.h"

#include <Engine/Engine.h>
#include <FileSystem/DynamicMemoryFile.h>
#include <FileSystem/Private/CheckIOError.h>
#include <DLCManager/DLCManager.h>
#include <UI/Focus/UIFocusComponent.h>
#include <UI/Render/UIDebugRenderComponent.h>
#include <UI/Update/UIUpdateComponent.h>
#include <EmbeddedWebServer/EmbeddedWebServer.h>
#include <DLCManager/DLCDownloader.h>

#include <fstream>

DLCManagerTest::DLCManagerTest(TestBed& app)
    : BaseScreen(app, "DLCManagerTest")
    , engine(app.GetEngine())
{
    GetOrCreateComponent<DAVA::UIUpdateComponent>();
}

DLCManagerTest::~DLCManagerTest()
{
    DAVA::DLCManager& dm = *engine.GetContext()->dlcManager;

    dm.requestUpdated.DisconnectAll();
    dm.networkReady.DisconnectAll();
    dm.Deinitialize();
}

void DLCManagerTest::TextFieldOnTextChanged(DAVA::UITextField* textField, const DAVA::WideString& newText, const DAVA::WideString& /*oldText*/)
{
    if (editUrl == textField)
    {
        urlToServerSuperpack = DAVA::UTF8Utils::EncodeToUTF8(newText);
    }
    UpdateDescription();
}

void DLCManagerTest::UpdateDescription()
{
    using namespace DAVA;

    const FilePath publicDocsPath = GetEngineContext()->fileSystem->GetPublicDocumentsPath();
    folderWithDownloadedPacks = publicDocsPath + "DLCManagerTest/packs/";

    const String packName = editPackName->GetUtf8Text();
    const String message = Format("status:\n\"%s\" - name of pack you want to download\n"
                                  "directory to download packs:\n%s\n"
                                  "Url to superpack.dvpk:\n%s\n",
                                  packName.c_str(),
                                  folderWithDownloadedPacks.GetAbsolutePathname().c_str(),
                                  urlToServerSuperpack.c_str());

    textStatusOutput->SetUtf8Text(message);
}

void DLCManagerTest::LoadResources()
{
    using namespace DAVA;
    profiler.Start();
    BaseScreen::LoadResources();

    const Color greyColor = Color(0.4f, 0.4f, 0.4f, 1.f);

    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));
    font->SetSize(20);

    editPackName = new UITextField(Rect(5, 10, 500, 40));
    editPackName->SetFont(font);
    editPackName->SetUtf8Text("all_level_packs");
    editPackName->SetFontSize(14);
    editPackName->GetOrCreateComponent<UIDebugRenderComponent>();
    editPackName->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    editPackName->SetInputEnabled(true);
    editPackName->GetOrCreateComponent<UIFocusComponent>();
    editPackName->SetDelegate(this);
    editPackName->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(editPackName);

    editUrl = new UITextField(Rect(5, 80, 500, 40));
    editUrl->SetFont(font);
    editUrl->SetFontSize(14);
    editUrl->SetText(UTF8Utils::EncodeToWideString(urlToServerSuperpack));
    editUrl->GetOrCreateComponent<UIDebugRenderComponent>();
    editUrl->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    editUrl->SetInputEnabled(true);
    editUrl->GetOrCreateComponent<UIFocusComponent>();
    editUrl->SetDelegate(this);
    editUrl->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(editUrl);

    progressRed = new UIControl(Rect(5, 150, 500, 5));
    progressRed->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Red);
    AddControl(progressRed);

    progressGreen = new UIControl(Rect(5, 150, 0, 5));
    progressGreen->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Green);
    AddControl(progressGreen);

    textStatusOutput = new UIStaticText(Rect(5, 190, 500, 500));
    textStatusOutput->SetFont(font);
    textStatusOutput->SetTextColor(Color::White);
    textStatusOutput->SetMultiline(true);
    textStatusOutput->SetUtf8Text("status output: ");
    textStatusOutput->GetOrCreateComponent<UIDebugRenderComponent>();
    textStatusOutput->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(textStatusOutput);

    buttonInitDLC = new UIButton(Rect(600, 10, 100, 40));
    buttonInitDLC->GetOrCreateComponent<UIDebugRenderComponent>();
    buttonInitDLC->SetStateFont(0xFF, font);
    buttonInitDLC->SetStateFontColor(0xFF, Color::White);
    buttonInitDLC->SetStateText(0xFF, L"Init");
    buttonInitDLC->SetStateFontColor(STATE_PRESSED_INSIDE, Color::Green);
    buttonInitDLC->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnInitClicked));
    AddControl(buttonInitDLC);

    buttonLoadPack = new UIButton(Rect(600, 80, 100, 40));
    buttonLoadPack->GetOrCreateComponent<UIDebugRenderComponent>();
    buttonLoadPack->SetStateFont(0xFF, font);
    buttonLoadPack->SetStateFontColor(0xFF, Color::White);
    buttonLoadPack->SetStateFontColor(STATE_PRESSED_INSIDE, Color::Green);
    buttonLoadPack->SetStateFontColor(STATE_DISABLED, greyColor);
    buttonLoadPack->SetStateText(0xFF, L"Load");
    buttonLoadPack->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnLoadClicked));
    buttonLoadPack->SetDisabled(true);
    AddControl(buttonLoadPack);

    buttonRemovePack = new UIButton(Rect(600, 150, 100, 40));
    buttonRemovePack->GetOrCreateComponent<UIDebugRenderComponent>();
    buttonRemovePack->SetStateFont(0xFF, font);
    buttonRemovePack->SetStateFontColor(0xFF, Color::White);
    buttonRemovePack->SetStateFontColor(STATE_PRESSED_INSIDE, Color::Green);
    buttonRemovePack->SetStateText(0xFF, L"Delete");
    buttonRemovePack->SetStateFontColor(STATE_DISABLED, greyColor);
    buttonRemovePack->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnDeleteClicked));
    buttonRemovePack->SetDisabled(true);
    AddControl(buttonRemovePack);

    buttonPauseResume = new UIButton(Rect(600, 220, 100, 40));
    buttonPauseResume->GetOrCreateComponent<UIDebugRenderComponent>();
    buttonPauseResume->SetStateFont(0xFF, font);
    buttonPauseResume->SetStateFontColor(0xFF, Color::White);
    buttonPauseResume->SetStateFontColor(STATE_PRESSED_INSIDE, Color::Green);
    buttonPauseResume->SetStateText(0xFF, L"Pause");
    buttonPauseResume->SetStateFontColor(STATE_DISABLED, greyColor);
    buttonPauseResume->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnOffRequestingClicked));
    buttonPauseResume->SetDisabled(true);
    AddControl(buttonPauseResume);

    UpdateDescription();
}

void DLCManagerTest::UnloadResources()
{
    using namespace DAVA;
    profiler.Stop();

    DLCManager& dm = *engine.GetContext()->dlcManager;
    dm.Deinitialize();

    SafeRelease(buttonPauseResume);
    SafeRelease(buttonRemovePack);
    SafeRelease(editPackName);
    SafeRelease(buttonLoadPack);
    SafeRelease(textStatusOutput);
    SafeRelease(progressRed);
    SafeRelease(progressGreen);
    SafeRelease(editUrl);
    SafeRelease(buttonInitDLC);

    BaseScreen::UnloadResources();
}

void DLCManagerTest::Update(DAVA::float32)
{
    using namespace DAVA;
    static DLCManager::Progress progress_;
    DLCManager& dm = *engine.GetContext()->dlcManager;
    {
        //DAVA_PROFILER_CPU_SCOPE_CUSTOM(__FUNCTION__, &profiler);
        DLCManager::Progress progress = dm.GetProgress();
        if (progress.isRequestingEnabled && progress.alreadyDownloaded != progress_.alreadyDownloaded)
        {
            //Logger::Info("progress: %d %d", static_cast<uint32>(progress.total), static_cast<uint32>(progress.alreadyDownloaded));
            progress_ = progress;
        }
    }
}

void DLCManagerTest::UpdateProgress(DAVA::float32 progress)
{
    auto rect = progressRed->GetRect();
    rect.dx = rect.dx * progress;
    progressGreen->SetRect(rect);
}

void DLCManagerTest::OnRequestUpdated(const DAVA::DLCManager::IRequest& request)
{
    using namespace DAVA;
    const String& packName = request.GetRequestedPackName();
    // change total download progress
    uint64 requestFileSize = request.GetSize();
    uint64 requestDownloadedSize = request.GetDownloadedSize();
    float32 requestProgress = requestFileSize > 0 ? static_cast<float32>(requestDownloadedSize) / requestFileSize : 1;

    std::stringstream ss;
    ss << packName << ": " << (requestProgress * 100) << '%';

    DLCManager& dm = *engine.GetContext()->dlcManager;
    auto p = dm.GetProgress();
    if (p.total > 0)
    {
        ss << "\ntotal:" << (100.0 * p.alreadyDownloaded) / p.total << '%';
    }

    std::string str = ss.str();
    textStatusOutput->SetUtf8Text(str);

    Logger::Info("DLC %s", str.c_str());

    UpdateProgress(requestProgress);
}

void DLCManagerTest::OnNetworkReady(bool isReady)
{
    using namespace DAVA;
    // To visualize on MacOS DownloadManager::Instance()->SetDownloadSpeedLimit(100000);
    // on MacOS slowly connect and then fast downloading
    std::stringstream ss;
    ss << "network ready = " << std::boolalpha << isReady;
    Logger::Info("%s", ss.str().c_str());

    textStatusOutput->SetUtf8Text("loading: " + ss.str());
}

static std::ostream& operator<<(std::ostream& stream, DAVA::DLCManager::InitStatus status)
{
    switch (status)
    {
    case DAVA::DLCManager::InitStatus::InProgress:
        stream << "not_finished";
        break;
    case DAVA::DLCManager::InitStatus::FinishedWithLocalMeta:
        stream << "using_local_meta";
        break;
    case DAVA::DLCManager::InitStatus::FinishedWithRemoteMeta:
        stream << "using_remote_meta";
        break;
    }
    return stream;
}

static std::ostream& operator<<(std::ostream& stream, DAVA::DLCManager::ErrorOrigin errOrigin)
{
    switch (errOrigin)
    {
    case DAVA::DLCManager::ErrorOrigin::FileIO:
        stream << "error_file_io";
        break;
    case DAVA::DLCManager::ErrorOrigin::InitTimeout:
        stream << "error_init_timeout";
        break;
    }
    return stream;
}

void DLCManagerTest::OnInitializeFinished(size_t numDownloaded, size_t numTotalFiles)
{
    buttonRemovePack->SetDisabled(false);
    buttonLoadPack->SetDisabled(false);
    buttonPauseResume->SetDisabled(false);

    DAVA::DLCManager& dm = *engine.GetContext()->dlcManager;
    std::stringstream ss;
    ss << "initialize finished: num_downloaded = " << numDownloaded << " num_total = " << numTotalFiles
       << "\ninit_status: " << dm.GetInitStatus() << std::endl;
    textStatusOutput->SetUtf8Text(ss.str());
}

void DLCManagerTest::OnErrorSignal(DAVA::DLCManager::ErrorOrigin errType, DAVA::int32 errnoVal, const DAVA::String& msg)
{
    std::stringstream ss;
    ss << "on_error_signal: msg: " << errType << " " << msg << " signal: " << errnoVal << " (" << strerror(errnoVal) << ")\n";
    //DAVA::String prev = logPring->GetUtf8Text();
    //logPring->SetUtf8Text(prev + ss.str());
}

void DLCManagerTest::OnInitClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    using namespace DAVA;
    DLCManager& dm = *engine.GetContext()->dlcManager;

    dm.Deinitialize();

    textStatusOutput->SetUtf8Text("done: start init");

    dm.networkReady.DisconnectAll();
    dm.networkReady.Connect(this, &DLCManagerTest::OnNetworkReady);
    dm.initializeFinished.Connect(this, &DLCManagerTest::OnInitializeFinished);
    dm.error.Connect(this, &DLCManagerTest::OnErrorSignal);

    DLCManager::Hints hints;
    FilePath publicDocsPath = GetEngineContext()->fileSystem->GetPublicDocumentsPath();
    hints.logFilePath = publicDocsPath.GetStringValue() + "dlc_manager_testbed.log";

    dm.Initialize(folderWithDownloadedPacks, urlToServerSuperpack, hints);

    dm.SetRequestingEnabled(true);

    textStatusOutput->SetUtf8Text("done: start initialize PackManager");
}

void DLCManagerTest::OnIOErrorClicked(BaseObject*, void*, void*)
{
    using namespace DAVA;
    DebugFS::IOErrorTypes ioErr;

    ioErr.moveFailed = true;
    ioErr.ioErrorCode = EACCES;

    GenerateIOErrorOnNextOperation(ioErr);
}

void DLCManagerTest::OnDeleteClicked(BaseObject* sender, void* data, void* callerData)
{
    using namespace DAVA;

    UpdateProgress(0.f);

    DLCManager& dm = *engine.GetContext()->dlcManager;

    String packName = editPackName->GetUtf8Text();
    dm.RemovePack(packName);
    textStatusOutput->SetUtf8Text("done: remove dvpk:" + packName);
}

void DLCManagerTest::OnListPacksClicked(BaseObject* sender, void* data, void* callerData)
{
    using namespace DAVA;
    DLCManager& dm = *engine.GetContext()->dlcManager;
    std::stringstream ss;
    String s = ss.str();

    textStatusOutput->SetText(UTF8Utils::EncodeToWideString(s));
}

void DLCManagerTest::OnOffRequestingClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    using namespace DAVA;
    DLCManager& dm = *engine.GetContext()->dlcManager;
    if (dm.IsRequestingEnabled())
    {
        dm.SetRequestingEnabled(false);
        buttonPauseResume->SetStateText(0xFF, L"Resume");
    }
    else
    {
        dm.SetRequestingEnabled(true);
        buttonPauseResume->SetStateText(0xFF, L"Pause");
    }
}

void DLCManagerTest::OnLoadClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    using namespace DAVA;

    DLCManager& dm = *engine.GetContext()->dlcManager;

    dm.requestUpdated.DisconnectAll();
    dm.requestUpdated.Connect(this, &DLCManagerTest::OnRequestUpdated);

    const String packName = editPackName->GetUtf8Text();

    try
    {
        if (dm.IsInitialized())
        {
            const DLCManager::IRequest* p = dm.RequestPack(packName);
            if (p != nullptr && p->IsDownloaded())
            {
                textStatusOutput->SetUtf8Text("already downloaded: " + packName);
                return;
            }
        }

        textStatusOutput->SetUtf8Text("loading: " + packName);
        dm.RequestPack(packName);
    }
    catch (Exception& ex)
    {
        textStatusOutput->SetUtf8Text(ex.what());
    }
}

void DLCManagerTest::OnExitButton(BaseObject* obj, void* data, void* callerData)
{
    using namespace DAVA;
    DLCManager& pm = *engine.GetContext()->dlcManager;
    pm.Deinitialize();

    BaseScreen::OnExitButton(obj, data, callerData);
}
