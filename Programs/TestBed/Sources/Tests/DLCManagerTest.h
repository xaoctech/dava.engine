#pragma once

#include "Infrastructure/BaseScreen.h"
#include <FileSystem/FilePath.h>
#include <DLCManager/DLCManager.h>
#include <Debug/ProfilerCPU.h>

class TestBed;
class DLCManagerTest : public BaseScreen, DAVA::UITextFieldDelegate
{
public:
    DLCManagerTest(TestBed& app);
    ~DLCManagerTest();

private:
    void TextFieldOnTextChanged(DAVA::UITextField* textField, const DAVA::WideString& newText, const DAVA::WideString& /*oldText*/) override;
    void UpdateDescription();

    void LoadResources() override;
    void UnloadResources() override;

    void Update(DAVA::float32 timeElapsed) override;

    void OnStartInitClicked(BaseObject* sender, void* data, void* callerData);
    void OnIOErrorClicked(BaseObject* sender, void* data, void* callerData);
    void OnClearDocsClicked(BaseObject* sender, void* data, void* callerData);
    void OnListPacksClicked(BaseObject* sender, void* data, void* callerData);
    void OnOffRequestingClicked(BaseObject* sender, void* data, void* callerData);
    void OnStartDownloadClicked(BaseObject* sender, void* data, void* callerData);
    void OnStartNextPackClicked(BaseObject* sender, void* data, void* callerData);
    void OnStartStopLocalServerClicked(BaseObject* sender, void* data, void* callerData);
    void OnCheckFileClicked(BaseObject* sender, void* data, void* callerData);
    void OnListInDvpkClicked(BaseObject* sender, void* data, void* callerData);
    void OnExitButton(BaseObject* obj, void* data, void* callerData) override;

    void WriteErrorOnDevice(DAVA::DLCManager::ErrorOrigin errType, DAVA::int32 errVal, const DAVA::String& filePath);
    void OnRequestUpdated(const DAVA::DLCManager::IRequest& request);
    void OnNetworkReady(bool isReady);
    void OnInitializeFinished(size_t numDownloaded, size_t numTotalFiles);
    void OnErrorSignal(DAVA::DLCManager::ErrorOrigin errType, DAVA::int32 errnoVal, const DAVA::String& msg);

    DAVA::Engine& engine;
    DAVA::ProfilerCPU profiler;

    DAVA::FilePath folderWithDownloadedPacks = "~doc:/DLCManagerTest/packs/";
    // TODO quick and dirty way to test download on all platforms, in future replace with local http server
    DAVA::String urlToServerSuperpack = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/smart_dlc/4.4.0.88_2035513_mali.dvpk";
    //"http://dl-wotblitz.wargaming.net/dlc/r11608713/3.7.0.236.dvpk";
    //"http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/smart_dlc/3.7.0.236.dvpk";
    //"http://dl-wotblitz.wargaming.net/dlc/r11608713/3.7.0.236.dvpk";
    //"http://127.0.0.1:8080/superpack_for_unittests.dvpk";
    //"http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/superpack_for_unittests.dvpk";
    //"http://127.0.0.1:2424/superpack.dvpk";

    DAVA::UIStaticText* packNameLoading = nullptr;
    DAVA::UIStaticText* logPring = nullptr;
    DAVA::UIButton* loadPack = nullptr;
    DAVA::UIButton* loadNext = nullptr;

    DAVA::UIButton* startServerButton = nullptr;
    DAVA::UIButton* stopServerButton = nullptr;

    DAVA::UITextField* packInput = nullptr;
    DAVA::UITextField* packNextInput = nullptr;
    DAVA::UITextField* numHandlesInput = nullptr;
    DAVA::UIControl* redControl = nullptr;
    DAVA::UIControl* greenControl = nullptr;
    DAVA::UIStaticText* description = nullptr;
    DAVA::UITextField* url = nullptr;
    DAVA::UITextField* filePathField = nullptr;
    DAVA::UIButton* checkFile = nullptr;
    DAVA::UIButton* startInit = nullptr;
    DAVA::UIButton* genIOError = nullptr;
    DAVA::UIButton* clearDocs = nullptr;
    DAVA::UIButton* lsDvpks = nullptr;
    DAVA::UIButton* OnOffRequesting = nullptr;
    DAVA::UITextField* dirToListFiles = nullptr;
    DAVA::UIButton* lsDirFromPacks = nullptr;
};
