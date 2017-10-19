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
    void OnExitButton(BaseObject* obj, void* data, void* callerData) override;

    void OnRequestUpdated(const DAVA::DLCManager::IRequest& request);
    void OnNetworkReady(bool isReady);
    void OnInitializeFinished(size_t numDownloaded, size_t numTotalFiles);
    void OnErrorSignal(DAVA::DLCManager::ErrorOrigin errType, DAVA::int32 errnoVal, const DAVA::String& msg);

    DAVA::Engine& engine;
    DAVA::ProfilerCPU profiler;

    DAVA::FilePath folderWithDownloadedPacks;
    // TODO quick and dirty way to test download on all platforms, in future replace with local http server
    DAVA::String urlToServerSuperpack = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/superpack_dx11.dvpk";
    //"http://dl-wotblitz.wargaming.net/dlc/r11608713/3.7.0.236.dvpk";
    //"http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/smart_dlc/3.7.0.236.dvpk";
    //"http://dl-wotblitz.wargaming.net/dlc/r11608713/3.7.0.236.dvpk";
    //"http://127.0.0.1:8080/superpack_for_unittests.dvpk";
    //"http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/superpack_for_unittests.dvpk";
    //"http://127.0.0.1:2424/superpack.dvpk";

    DAVA::UIStaticText* textProperties = nullptr;
    DAVA::UIButton* buttonLoadPack = nullptr;

    DAVA::UITextField* editPackName = nullptr;
    DAVA::UIControl* progressRed = nullptr;
    DAVA::UIControl* progressGreen = nullptr;
    DAVA::UIStaticText* textDescription = nullptr;
    DAVA::UITextField* editUrl = nullptr;
    DAVA::UIButton* buttonInitDLC = nullptr;
    DAVA::UIButton* buttonRemoveDoc = nullptr;
};
