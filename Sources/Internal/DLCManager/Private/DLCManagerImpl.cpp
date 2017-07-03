#include "DLCManager/Private/DLCManagerImpl.h"
#include "FileSystem/FileList.h"
#include "FileSystem/File.h"
#include "FileSystem/Private/PackArchive.h"
#include "FileSystem/Private/PackMetaData.h"
#include "FileSystem/FileAPIHelper.h"
#include "DLCManager/DLCDownloader.h"
#include "Utils/CRC32.h"
#include "Logger/Logger.h"
#include "Base/Exception.h"
#include "Time/SystemTimer.h"
#include "Engine/Engine.h"
#include "Debug/Backtrace.h"
#include "Platform/DeviceInfo.h"
#include "DLCManager/Private/PackRequest.h"

#include <iomanip>

namespace DAVA
{
DLCManager::~DLCManager() = default;
DLCManager::IRequest::~IRequest() = default;

const String& DLCManagerImpl::ToString(InitState state)
{
    static const Vector<String> states{
        "Starting",
        "LoadingRequestAskFooter",
        "LoadingRequestGetFooter",
        "LoadingRequestAskFileTable",
        "LoadingRequestGetFileTable",
        "CalculateLocalDBHashAndCompare",
        "LoadingRequestAskMeta",
        "LoadingRequestGetMeta",
        "UnpakingDB",
        "LoadingPacksDataFromLocalMeta",
        "WaitScanThreadToFinish",
        "MoveDeleyedRequestsToQueue",
        "Ready",
        "Offline"
    };
    DVASSERT(states.size() == static_cast<uint32>(InitState::State_COUNT));
    return states.at(static_cast<size_t>(state));
}

static void WriteBufferToFile(const Vector<uint8>& outDB, const FilePath& path)
{
    ScopedPtr<File> f(File::Create(path, File::WRITE | File::CREATE));
    if (!f)
    {
        DAVA_THROW(DAVA::Exception, "can't create file for local DB: " + path.GetStringValue());
    }

    uint32 written = f->Write(outDB.data(), static_cast<uint32>(outDB.size()));
    if (written != outDB.size())
    {
        DAVA_THROW(DAVA::Exception, "can't write file for local DB: " + path.GetStringValue());
    }
}

std::ostream& DLCManagerImpl::GetLog() const
{
    return log;
}

DLCManagerImpl::DLCManagerImpl(Engine* engine_)
    : engine(*engine_)
{
    DVASSERT(Thread::IsMainThread());
    engine.update.Connect(this, [this](float32 frameDelta)
                          {
                              Update(frameDelta, false);
                          });
    engine.backgroundUpdate.Connect(this, [this](float32 frameDelta)
                                    {
                                        Update(frameDelta, true);
                                    });
}

void DLCManagerImpl::ClearResouces()
{
    if (scanThread)
    {
        scanThread->Cancel();
        metaDataLoadedSem.Post();
        if (scanThread->IsJoinable())
        {
            scanThread->Join();
        }
        scanThread->Release();
        scanThread = nullptr;
    }

    for (auto request : requests)
    {
        delete request;
    }

    requests.clear();

    for (auto request : delayedRequests)
    {
        delete request;
    }

    initState = InitState::Starting;
    delayedRequests.clear();
    meta.reset();
    requestManager.reset();

    buffer.clear();
    uncompressedFileNames.clear();
    mapFileData.clear();
    startFileNameIndexesInUncompressedNames.clear();

    if (downloadTask != nullptr)
    {
        if (downloader != nullptr)
        {
            downloader->RemoveTask(downloadTask);
            downloadTask = nullptr;
        }
    }

    downloader.reset(nullptr);

    fullSizeServerData = 0;

    timeWaitingNextInitializationAttempt = 0;
    retryCount = 0;
}

DLCManagerImpl::~DLCManagerImpl()
{
    DVASSERT(Thread::IsMainThread());

    engine.update.Disconnect(this);
    engine.backgroundUpdate.Disconnect(this);

    ClearResouces();
}

void DLCManagerImpl::TestWriteAccessToPackDirectory(const FilePath& dirToDownloadPacks_)
{
    FilePath tmpFile = dirToDownloadPacks_ + "tmp.file";
    {
        ScopedPtr<File> f(File::Create(tmpFile, File::WRITE | File::CREATE));
        if (!f)
        {
            String err = "can't write into directory: " + dirToDownloadedPacks.GetStringValue();
            DAVA_THROW(DAVA::Exception, err);
        }
    }
    FileSystem* fs = GetEngineContext()->fileSystem;
    fs->DeleteFile(tmpFile);
}

void DLCManagerImpl::FillPreloadedPacks()
{
    preloadedPacks.clear();
    if (!hints.preloadedPacks.empty())
    {
        DVASSERT(hints.preloadedPacks.find(' ') == String::npos); // No spaces

        StringStream ss(hints.preloadedPacks);
        for (String packName; getline(ss, packName);)
        {
            if (packName.empty())
            {
                continue; // skip empty lines if any
            }
            preloadedPacks.emplace(packName, PreloadedPack(packName));
        }
    }
}

void DLCManagerImpl::TestPackDirectoryExist()
{
    FileSystem* fs = GetEngineContext()->fileSystem;
    if (FileSystem::DIRECTORY_CANT_CREATE == fs->CreateDirectory(dirToDownloadedPacks, true))
    {
        String err = "can't create directory for packs: " + dirToDownloadedPacks.GetStringValue();
        DAVA_THROW(DAVA::Exception, err);
    }
}

void DLCManagerImpl::DumpInitialParams(const FilePath& dirToDownloadPacks, const String& urlToServerSuperpack, const Hints& hints_)
{
    if (!log.is_open())
    {
        FilePath p(hints_.logFilePath);
        String fullLogPath = p.GetAbsolutePathname();

        log.open(fullLogPath.c_str(), std::ios::trunc);
        if (!log)
        {
            const char* err = strerror(errno);
            Logger::Error("can't create dlc_manager.log error: %s", err);
            DAVA_THROW(DAVA::Exception, err);
        }

        String preloaded = hints_.preloadedPacks;
        transform(begin(preloaded), end(preloaded), begin(preloaded), [](char c)
                  {
                      return c == '\n' ? ' ' : c;
                  });

        log << "DLCManager::Initialize" << '\n'
            << "(\n"
            << "    dirToDownloadPacks: " << dirToDownloadPacks.GetAbsolutePathname() << '\n'
            << "    urlToServerSuperpack: " << urlToServerSuperpack << '\n'
            << "    hints:\n"
            << "    (\n"
            << "        logFilePath(this file): " << hints_.logFilePath << '\n'
            << "        preloadedPacks: " << preloaded << '\n'
            << "        retryConnectMilliseconds: " << hints_.retryConnectMilliseconds << '\n'
            << "        maxFilesToDownload: " << hints_.maxFilesToDownload << '\n'
            << "        timeoutForDownload: " << hints_.timeoutForDownload << '\n'
            << "        skipCDNConnectAfterAttemps: " << hints_.skipCDNConnectAfterAttempts << '\n'
            << "        downloaderMaxHandles: " << hints_.downloaderMaxHandles << '\n'
            << "        downloaderChankBufSize: " << hints_.downloaderChunkBufSize << '\n'
            << "    )\n"
            << ")\n";

        const EnumMap* enumMap = GlobalEnumMap<eGPUFamily>::Instance();
        eGPUFamily e = DeviceInfo::GetGPUFamily();
        const char* gpuFamily = enumMap->ToString(e);
        log << "current_device_gpu: " << gpuFamily << std::endl;
    }
}

void DLCManagerImpl::CreateDownloader()
{
    if (!downloader)
    {
        DLCDownloader::Hints downloaderHints;
        downloaderHints.numOfMaxEasyHandles = static_cast<int>(hints.downloaderMaxHandles);
        downloaderHints.chunkMemBuffSize = static_cast<int>(hints.downloaderChunkBufSize);
        downloaderHints.timeout = static_cast<int>(hints.timeoutForDownload);

        downloader.reset(DLCDownloader::Create());
        downloader->SetHints(downloaderHints);
    }
}

void DLCManagerImpl::Initialize(const FilePath& dirToDownloadPacks_,
                                const String& urlToServerSuperpack_,
                                const Hints& hints_)
{
    DVASSERT(Thread::IsMainThread());

    bool isFirstTimeCall = (log.is_open() == false);

    DumpInitialParams(dirToDownloadPacks_, urlToServerSuperpack_, hints_);

    log << __FUNCTION__ << std::endl;

    CreateDownloader();

    if (!IsInitialized())
    {
        dirToDownloadedPacks = dirToDownloadPacks_;
        localCacheMeta = dirToDownloadPacks_ + "local_copy_server_meta.meta";
        localCacheFileTable = dirToDownloadPacks_ + "local_copy_server_file_table.block";
        localCacheFooter = dirToDownloadPacks_ + "local_copy_server_footer.footer";
        urlToSuperPack = urlToServerSuperpack_;
        hints = hints_;

        TestPackDirectoryExist();
        TestWriteAccessToPackDirectory(dirToDownloadPacks_);
        FillPreloadedPacks();
    }

    // if Initialize called second time
    fullSizeServerData = 0;
    if (nullptr != downloadTask)
    {
        downloader->RemoveTask(downloadTask);
        downloadTask = nullptr;
    }

    initState = InitState::LoadingRequestAskFooter;

    // safe to call several times, only first will work
    StartScanDownloadedFiles();

    if (isFirstTimeCall)
    {
        SetRequestingEnabled(true);
    }
}

void DLCManagerImpl::Deinitialize()
{
    DVASSERT(Thread::IsMainThread());

    log << __FUNCTION__ << std::endl;

    if (IsInitialized())
    {
        SetRequestingEnabled(false);
    }

    ClearResouces();

    log.close();
}

bool DLCManagerImpl::IsInitialized() const
{
    DVASSERT(Thread::IsMainThread());
    return nullptr != requestManager && delayedRequests.empty() && scanThread == nullptr;
}

DLCManagerImpl::InitState DLCManagerImpl::GetInitState() const
{
    DVASSERT(Thread::IsMainThread());
    return initState;
}

const String& DLCManagerImpl::GetLastErrorMessage() const
{
    DVASSERT(Thread::IsMainThread());
    return initErrorMsg;
}

// end Initialization ////////////////////////////////////////

void DLCManagerImpl::Update(float frameDelta, bool inBackground)
{
    DVASSERT(Thread::IsMainThread());

    try
    {
        if (InitState::Starting != initState)
        {
            if (initState != InitState::Ready)
            {
                ContinueInitialization(frameDelta);
            }
            else if (isProcessingEnabled)
            {
                if (requestManager)
                {
                    requestManager->Update(inBackground);
                }
            }
        }
    }
    catch (Exception& ex)
    {
        log << "PackManager error: exception: " << ex.what() << " file: " << ex.file << "(" << ex.line << ")" << std::endl;
        Logger::Error("PackManager error: %s", ex.what());
        throw; // crush or let parent code decide
    }
}

void DLCManagerImpl::WaitScanThreadToFinish()
{
    if (scanState == ScanState::Done)
    {
        initState = InitState::MoveDeleyedRequestsToQueue;
    }
}

void DLCManagerImpl::ContinueInitialization(float frameDelta)
{
    if (timeWaitingNextInitializationAttempt > 0.f)
    {
        timeWaitingNextInitializationAttempt -= frameDelta;
        if (timeWaitingNextInitializationAttempt <= 0.f)
        {
            timeWaitingNextInitializationAttempt = 0.f;
        }
        else
        {
            return;
        }
    }

    if (InitState::Starting == initState)
    {
        initState = InitState::LoadingRequestAskFooter;
    }
    else if (InitState::LoadingRequestAskFooter == initState)
    {
        AskFooter();
    }
    else if (InitState::LoadingRequestGetFooter == initState)
    {
        GetFooter();
    }
    else if (InitState::LoadingRequestAskFileTable == initState)
    {
        AskFileTable();
    }
    else if (InitState::LoadingRequestGetFileTable == initState)
    {
        GetFileTable();
    }
    else if (InitState::CalculateLocalDBHashAndCompare == initState)
    {
        CompareLocalMetaWitnRemoteHash();
    }
    else if (InitState::LoadingRequestAskMeta == initState)
    {
        AskServerMeta();
    }
    else if (InitState::LoadingRequestGetMeta == initState)
    {
        GetServerMeta();
    }
    else if (InitState::UnpakingDB == initState)
    {
        ParseMeta();
    }
    else if (InitState::LoadingPacksDataFromLocalMeta == initState)
    {
        LoadPacksDataFromMeta();
    }
    else if (InitState::WaitScanThreadToFinish == initState)
    {
        WaitScanThreadToFinish();
    }
    else if (InitState::MoveDeleyedRequestsToQueue == initState)
    {
        StartDelayedRequests();
    }
    else if (InitState::Ready == initState)
    {
        // happy end
    }
}

PackRequest* DLCManagerImpl::AddDelayedRequest(const String& requestedPackName)
{
    for (auto* request : delayedRequests)
    {
        if (request->GetRequestedPackName() == requestedPackName)
        {
            return request;
        }
    }

    delayedRequests.push_back(new PackRequest(*this, requestedPackName));
    return delayedRequests.back();
}

PackRequest* DLCManagerImpl::CreateNewRequest(const String& requestedPackName)
{
    for (auto* request : requests)
    {
        if (request->GetRequestedPackName() == requestedPackName)
        {
            return request;
        }
    }

    log << __FUNCTION__ << " requestedPackName: " << requestedPackName << std::endl;

    Vector<uint32> packIndexes = meta->GetFileIndexes(requestedPackName);

    // check all requested files already downloaded
    auto isFileDownloaded = [&](uint32 index) { return IsFileReady(index); };
    auto removeIt = remove_if(begin(packIndexes), end(packIndexes), isFileDownloaded);
    if (removeIt != end(packIndexes))
    {
        packIndexes.erase(removeIt, end(packIndexes));
    }

    PackRequest* request = new PackRequest(*this, requestedPackName, std::move(packIndexes));

    Vector<uint32> deps = request->GetDependencies();

    for (uint32 dependent : deps)
    {
        const String& depPackName = meta->GetPackInfo(dependent).packName;
        PackRequest* r = FindRequest(depPackName);
        if (nullptr == r)
        {
            // recursive call
            PackRequest* dependentRequest = CreateNewRequest(depPackName);
            DVASSERT(dependentRequest != nullptr);
        }
    }

    requests.push_back(request);
    requestManager->Push(request);

    return request;
}

bool DLCManagerImpl::IsLocalMetaAlreadyExist() const
{
    FileSystem* fs = engine.GetContext()->fileSystem;
    bool localFileTableExist = fs->IsFile(localCacheFileTable);
    bool localMetaExist = fs->IsFile(localCacheMeta);
    return localFileTableExist && localMetaExist;
}

void DLCManagerImpl::TestRetryCountLocalMetaAndGoTo(InitState nextState, InitState alternateState)
{
    ++retryCount;
    timeWaitingNextInitializationAttempt = hints.retryConnectMilliseconds / 1000.f;

    if (retryCount > hints.skipCDNConnectAfterAttempts)
    {
        if (IsLocalMetaAlreadyExist())
        {
            initState = nextState;
        }
        else
        {
            initState = alternateState;
        }
    }
    else
    {
        initState = alternateState;
    }
}

void DLCManagerImpl::FireNetworkReady(bool nextState)
{
    if (nextState != prevNetworkState || firstTimeNetworkState == false)
    {
        networkReady.Emit(nextState);
        prevNetworkState = nextState;
        firstTimeNetworkState = true;
    }
}

void DLCManagerImpl::AskFooter()
{
    //Logger::FrameworkDebug("pack manager ask_footer");

    DVASSERT(0 == fullSizeServerData);

    if (nullptr == downloadTask)
    {
        downloadTask = downloader->StartGetContentSize(urlToSuperPack);
        if (nullptr == downloadTask)
        {
            DAVA_THROW(Exception, "can't start get_size task with url: " + urlToSuperPack);
        }
    }
    else
    {
        DLCDownloader::TaskStatus status = downloader->GetTaskStatus(downloadTask);

        if (DLCDownloader::TaskState::Finished == status.state)
        {
            downloader->RemoveTask(downloadTask);
            downloadTask = nullptr;

            bool allGood = !status.error.errorHappened;
            if (allGood)
            {
                retryCount = 0;
                fullSizeServerData = status.sizeTotal;
                if (fullSizeServerData < sizeof(PackFormat::PackFile))
                {
                    log << "error: too small superpack on server: " << status << std::endl;
                    DAVA_THROW(DAVA::Exception, "too small superpack on server fullSizeServerData:");
                }
                // start downloading footer from server superpack
                uint64 downloadOffset = fullSizeServerData - sizeof(initFooterOnServer);
                uint32 sizeofFooter = static_cast<uint32>(sizeof(initFooterOnServer));

                memBufWriter.reset(new MemoryBufferWriter(&initFooterOnServer, sizeofFooter));
                downloadTask = downloader->StartTask(urlToSuperPack, *memBufWriter, DLCDownloader::Range(downloadOffset, sizeofFooter));
                if (nullptr == downloadTask)
                {
                    DAVA_THROW(Exception, "can't start get_size task with url: " + urlToSuperPack);
                }
                initState = InitState::LoadingRequestGetFooter;
                log << "initState: " << ToString(initState) << std::endl;

                FireNetworkReady(true);
            }
            else
            {
                initErrorMsg = "failed get superpack size on server, download error: ";
                log << initErrorMsg << " " << status << std::endl;

                FireNetworkReady(false);

                TestRetryCountLocalMetaAndGoTo(InitState::LoadingPacksDataFromLocalMeta, InitState::LoadingRequestAskFooter);
            }
        }
    }
}

void DLCManagerImpl::SaveServerFooter()
{
    ScopedPtr<File> f(File::Create(localCacheFooter, File::CREATE | File::WRITE));
    if (f)
    {
        if (sizeof(initFooterOnServer) == f->Write(&initFooterOnServer, sizeof(initFooterOnServer)))
        {
            return; // all good
        }
    }

    StringStream ss;
    ss << "can't write file: " << localCacheFooter.GetAbsolutePathname() << " errno: (" << errno << ") " << strerror(errno) << std::endl;
    log << ss.str();
    fileErrorOccured.Emit(localCacheFooter.GetAbsolutePathname(), errno);
    DAVA_THROW(Exception, ss.str());
}

void DLCManagerImpl::GetFooter()
{
    //Logger::FrameworkDebug("pack manager get_footer");

    DLCDownloader::TaskStatus status = downloader->GetTaskStatus(downloadTask);

    if (DLCDownloader::TaskState::Finished == status.state)
    {
        downloader->RemoveTask(downloadTask);
        downloadTask = nullptr;

        bool allGood = !status.error.errorHappened;
        if (allGood)
        {
            retryCount = 0;
            uint32 crc32 = CRC32::ForBuffer(reinterpret_cast<char*>(&initFooterOnServer.info), sizeof(initFooterOnServer.info));
            if (crc32 != initFooterOnServer.infoCrc32)
            {
                log << "error: on server bad superpack!!! Footer not match crc32 "
                    << std::hex << crc32 << " != " << std::hex
                    << initFooterOnServer.infoCrc32 << std::dec << std::endl;
                DAVA_THROW(DAVA::Exception, "on server bad superpack!!! Footer not match crc32");
            }

            SaveServerFooter();

            usedPackFile.footer = initFooterOnServer;
            initState = InitState::LoadingRequestAskFileTable;
            log << "initState: " << ToString(initState) << std::endl;

            FireNetworkReady(true);
        }
        else
        {
            initErrorMsg = "failed get footer from server, download error: ";
            log << initErrorMsg << " " << status << std::endl;

            FireNetworkReady(false);

            TestRetryCountLocalMetaAndGoTo(InitState::LoadingPacksDataFromLocalMeta, InitState::LoadingRequestAskFooter);
        }
    }
}

void DLCManagerImpl::AskFileTable()
{
    //Logger::FrameworkDebug("pack manager ask_file_table");

    FileSystem* fs = engine.GetContext()->fileSystem;
    if (fs->IsFile(localCacheFileTable))
    {
        uint32 crc = CRC32::ForFile(localCacheFileTable);
        if (crc == initFooterOnServer.info.filesTableCrc32)
        {
            initState = InitState::LoadingRequestGetFileTable;
            return;
        }
        fs->DeleteFile(localCacheFileTable);
    }

    buffer.resize(initFooterOnServer.info.filesTableSize);

    uint64 downloadOffset = fullSizeServerData - (sizeof(initFooterOnServer) + initFooterOnServer.info.filesTableSize);

    downloadTask = downloader->StartTask(urlToSuperPack, localCacheFileTable.GetAbsolutePathname(), DLCDownloader::Range(downloadOffset, buffer.size()));
    if (nullptr == downloadTask)
    {
        DAVA_THROW(DAVA::Exception, "can't start downloading into buffer");
    }
    initState = InitState::LoadingRequestGetFileTable;
    log << "initState: " << ToString(initState) << std::endl;
}

void DLCManagerImpl::ReadLocalFileTableInfoBuffer()
{
    uint64 fileSize = 0;
    FileSystem* fs = engine.GetContext()->fileSystem;

    fs->GetFileSize(localCacheFileTable, fileSize);

    buffer.resize(static_cast<size_t>(fileSize));

    ScopedPtr<File> f(File::Create(localCacheFileTable, File::OPEN | File::READ));
    if (f)
    {
        f->Read(buffer.data(), static_cast<uint32>(buffer.size()));
    }
    else
    {
        log << "error: failed open file: " << localCacheFileTable.GetAbsolutePathname() << " (" << errno << ") " << strerror(errno) << std::endl;
        DAVA_THROW(Exception, "can't open file: " + localCacheFileTable.GetAbsolutePathname());
    }
}

void DLCManagerImpl::FillFileNameIndexes()
{
    startFileNameIndexesInUncompressedNames.clear();
    startFileNameIndexesInUncompressedNames.reserve(usedPackFile.filesTable.data.files.size());
    startFileNameIndexesInUncompressedNames.push_back(0); // first name, and skip last '\0' char
    for (uint32 index = 0, last = static_cast<uint32>(uncompressedFileNames.size()) - 1;
         index < last; ++index)
    {
        if (uncompressedFileNames[index] == '\0')
        {
            startFileNameIndexesInUncompressedNames.push_back(index + 1);
        }
    }
}

void DLCManagerImpl::ReadContentAndExtractFileNames()
{
    ReadLocalFileTableInfoBuffer();

    uint32 crc32 = CRC32::ForBuffer(buffer);
    if (crc32 != initFooterOnServer.info.filesTableCrc32)
    {
        const char* err = "FileTable not match crc32";
        log << "error: " << err << std::endl;
        DAVA_THROW(DAVA::Exception, err);
    }

    uncompressedFileNames.clear();
    PackArchive::ExtractFileTableData(initFooterOnServer,
                                      buffer,
                                      uncompressedFileNames,
                                      usedPackFile.filesTable);

    FillFileNameIndexes();

    initState = InitState::CalculateLocalDBHashAndCompare;
    log << "initState: " << ToString(initState) << std::endl;
}

void DLCManagerImpl::GetFileTable()
{
    //Logger::FrameworkDebug("pack manager get_file_table");

    DLCDownloader::TaskStatus status;

    if (downloadTask != nullptr)
    {
        status = downloader->GetTaskStatus(downloadTask);
        if (DLCDownloader::TaskState::Finished == status.state)
        {
            downloader->RemoveTask(downloadTask);
            downloadTask = nullptr;

            bool allGood = !status.error.errorHappened;
            if (allGood)
            {
                retryCount = 0;
                ReadContentAndExtractFileNames();

                FireNetworkReady(true);
            }
            else
            {
                initErrorMsg = "failed get fileTable from server, download error: ";
                log << "error: " << initErrorMsg << std::endl;

                FireNetworkReady(false);

                TestRetryCountLocalMetaAndGoTo(InitState::LoadingPacksDataFromLocalMeta, InitState::LoadingRequestAskFileTable);
            }
        }
    }
    else
    {
        // we already have file without additional request
        ReadContentAndExtractFileNames();
    }
}

void DLCManagerImpl::CompareLocalMetaWitnRemoteHash()
{
    //Logger::FrameworkDebug("pack manager calc_local_db_with_remote_crc32");

    FileSystem* fs = engine.GetContext()->fileSystem;

    if (fs->IsFile(localCacheMeta))
    {
        const uint32 localCrc32 = CRC32::ForFile(localCacheMeta);
        if (localCrc32 != initFooterOnServer.metaDataCrc32)
        {
            DeleteLocalMetaFiles();
            // we have to download new localDB file from server!
            initState = InitState::LoadingRequestAskMeta;
        }
        else
        {
            // all good go to
            initState = InitState::LoadingPacksDataFromLocalMeta;
        }
    }
    else
    {
        DeleteLocalMetaFiles();

        initState = InitState::LoadingRequestAskMeta;
    }
    log << "initState: " << ToString(initState) << std::endl;
}

void DLCManagerImpl::AskServerMeta()
{
    //Logger::FrameworkDebug("pack manager ask_db");

    uint64 internalDataSize = initFooterOnServer.metaDataSize +
    initFooterOnServer.info.filesTableSize +
    sizeof(PackFormat::PackFile::FooterBlock);

    uint64 downloadOffset = fullSizeServerData - internalDataSize;
    uint64 downloadSize = initFooterOnServer.metaDataSize;

    buffer.resize(static_cast<size_t>(downloadSize));

    memBufWriter.reset(new MemoryBufferWriter(buffer.data(), buffer.size()));

    downloadTask = downloader->StartTask(urlToSuperPack, *memBufWriter, DLCDownloader::Range(downloadOffset, downloadSize));
    if (nullptr == downloadTask)
    {
        DAVA_THROW(Exception, "can't start download task into memory buffer");
    }

    initState = InitState::LoadingRequestGetMeta;
    log << "initState: " << ToString(initState) << std::endl;
}

void DLCManagerImpl::GetServerMeta()
{
    //Logger::FrameworkDebug("pack manager get_db");

    DLCDownloader::TaskStatus status = downloader->GetTaskStatus(downloadTask);

    if (DLCDownloader::TaskState::Finished == status.state)
    {
        downloader->RemoveTask(downloadTask);
        downloadTask = nullptr;

        bool allGood = !status.error.errorHappened;
        if (allGood)
        {
            retryCount = 0;
            initState = InitState::UnpakingDB;
            log << "initState: " << ToString(initState) << std::endl;

            FireNetworkReady(true);
        }
        else
        {
            initErrorMsg = "failed get meta from server, download error: ";
            log << initErrorMsg << status << std::endl;

            FireNetworkReady(false);

            TestRetryCountLocalMetaAndGoTo(InitState::LoadingPacksDataFromLocalMeta, InitState::LoadingRequestAskMeta);
        }
    }
}

void DLCManagerImpl::ParseMeta()
{
    //Logger::FrameworkDebug("pack manager unpacking_db");

    uint32 buffCrc32 = CRC32::ForBuffer(buffer);

    try
    {
        if (buffCrc32 != initFooterOnServer.metaDataCrc32)
        {
            log << "on server bad superpack!!! Footer meta not match crc32 "
                << std::hex << buffCrc32 << " != "
                << initFooterOnServer.metaDataCrc32 << std::dec << std::endl;
            DAVA_THROW(Exception, "on server bad superpack!!! Footer meta not match crc32");
        }

        WriteBufferToFile(buffer, localCacheMeta);
    }
    catch (Exception& ex)
    {
        int32 localErrno = errno;
        log << "error: " << ex.what() << " file: " << ex.file << "("
            << ex.line << ") errno: (" << localErrno << ") "
            << strerror(localErrno) << std::endl;

        fileErrorOccured.Emit(localCacheMeta.GetAbsolutePathname(), localErrno);

        // lets start all over again
        initState = InitState::LoadingRequestAskFooter;
        return;
    }

    buffer.clear();
    buffer.shrink_to_fit();

    initState = InitState::LoadingPacksDataFromLocalMeta;
    log << "initState: " << ToString(initState) << std::endl;
}

void DLCManagerImpl::LoadLocalCacheServerFooter()
{
    ScopedPtr<File> f(File::Create(localCacheFooter, File::OPEN | File::READ));
    if (f)
    {
        if (sizeof(initFooterOnServer) == f->Read(&initFooterOnServer, sizeof(initFooterOnServer)))
        {
            return;
        }
    }

    log << "can't read file: " << localCacheFooter.GetAbsolutePathname() << " errno(" << errno << ") error: " << strerror(errno) << std::endl;
    DAVA_THROW(Exception, "can't load localCacheFooter data");
}

void DLCManagerImpl::LoadPacksDataFromMeta()
{
    //Logger::FrameworkDebug("pack manager load_packs_data_from_db");

    try
    {
        if (initFooterOnServer.info.packArchiveMarker == Array<char8, 4>{ '\0', '\0', '\0', '\0' })
        {
            // no server data, so use local as is (preload existing file_names_cache)
            LoadLocalCacheServerFooter();

            ReadContentAndExtractFileNames();
        }

        ScopedPtr<File> f(File::Create(localCacheMeta, File::OPEN | File::READ));

        uint32 size = static_cast<uint32>(f->GetSize());

        buffer.resize(size);

        uint32 readSize = f->Read(buffer.data(), size);

        if (size != readSize)
        {
            DAVA_THROW(Exception, "can't read localCacheMeta size not match");
        }

        uint32 buffHash = CRC32::ForBuffer(buffer);

        if (initFooterOnServer.metaDataCrc32 != buffHash)
        {
            DAVA_THROW(Exception, "can't read localCacheMeta hash not match");
        }

        meta.reset(new PackMetaData(buffer.data(), buffer.size()));

        size_t numFiles = meta->GetFileCount();
        scanFileReady.resize(numFiles);

        // now user can do requests for local packs
        requestManager.reset(new RequestManager(*this));
    }
    catch (Exception& ex)
    {
        log << "can't load pack data from meta: " << ex.what() << " file: " << ex.file << "(" << ex.line << ")" << std::endl;
        engine.GetContext()->fileSystem->DeleteFile(localCacheMeta);

        // lets start all over again
        initState = InitState::LoadingRequestAskFooter;
        return;
    }

    metaDataLoadedSem.Post();

    initState = InitState::WaitScanThreadToFinish;
    log << "initState: " << ToString(initState) << std::endl;
}

void DLCManagerImpl::SwapPointers(PackRequest* userRequestObject, PackRequest* invalidPointer)
{
    auto it = find(begin(requests), end(requests), invalidPointer);
    DVASSERT(it != end(requests));
    // change old pointer (just deleted) to correct one
    *it = userRequestObject;
}

void DLCManagerImpl::SwapRequestAndUpdatePointers(PackRequest* userRequestObject, PackRequest* newRequestObject)
{
    // We want to give user same pointer, so we need move(swap) old object
    // with new object value
    *userRequestObject = std::move(*newRequestObject);
    delete newRequestObject; // now this pointer is invalid!
    PackRequest* invalidPointer = newRequestObject;
    // so we have to update all references to new swapped pointer value
    // find new pointer and change it to correct one
    SwapPointers(userRequestObject, invalidPointer);

    requestManager->SwapPointers(userRequestObject, invalidPointer);
}

void DLCManagerImpl::StartDelayedRequests()
{
    //Logger::FrameworkDebug("pack manager mount_downloaded_packs");
    if (scanThread != nullptr)
    {
        // scan thread should be finished already
        if (scanThread->IsJoinable())
        {
            scanThread->Join();
        }
        scanThread->Release();
        scanThread = nullptr;
    }

    // I want to create new requests then move its content to old
    // to save pointers for users, if user store pointer to IRequest
    Vector<PackRequest*> tmpRequests;
    delayedRequests.swap(tmpRequests);
    // first remove old request pointers from requestManager
    for (auto request : tmpRequests)
    {
        requestManager->Remove(request);
    }

    for (PackRequest* request : tmpRequests)
    {
        const String& requestedPackName = request->GetRequestedPackName();
        PackRequest* r = FindRequest(requestedPackName);

        if (r == nullptr)
        {
            PackRequest* newRequest = CreateNewRequest(requestedPackName);
            DVASSERT(newRequest != request);
            DVASSERT(newRequest != nullptr);

            SwapRequestAndUpdatePointers(request, newRequest);
        }
        else
        {
            DVASSERT(r != request);
            // if we come here, it means one of previous requests
            // create it's dependencies and this is it
            SwapRequestAndUpdatePointers(request, r);
        }
    }

    initState = InitState::Ready;
    log << "initState: " << ToString(initState) << std::endl;

    size_t numDownloaded = count(begin(scanFileReady), end(scanFileReady), true);

    initializeFinished.Emit(numDownloaded, meta->GetFileCount());

    for (PackRequest* request : tmpRequests)
    {
        // we have to inform user because after scanning is finished
        // some request may be already downloaded (all files found)
        requestUpdated.Emit(*request);
    }
}

void DLCManagerImpl::DeleteLocalMetaFiles()
{
    FileSystem* fs = engine.GetContext()->fileSystem;
    fs->DeleteFile(localCacheMeta);
}

bool DLCManagerImpl::IsPackDownloaded(const String& packName)
{
    DVASSERT(Thread::IsMainThread());

    if (end(preloadedPacks) != preloadedPacks.find(packName))
    {
        return true;
    }

    if (!IsInitialized())
    {
        DVASSERT(false && "Initialization not finished. Files is scanning now.");
        log << "Initialization not finished. Files is scanning now." << std::endl;
        return false;
    }
    // client wants only assert on bad pack name or dependency name
    try
    {
        // check every file in requested pack and all it's dependencies
        Vector<uint32> packFileIndexes = meta->GetFileIndexes(packName);
        for (uint32 fileIndex : packFileIndexes)
        {
            if (!IsFileReady(fileIndex))
            {
                return false;
            }
        }

        Vector<uint32> deps = meta->GetPackDependencyIndexes(packName);

        for (uint32 dependencyPack : deps)
        {
            const String& depPackName = meta->GetPackInfo(dependencyPack).packName;
            if (!IsPackDownloaded(depPackName)) // recursive call
            {
                return false;
            }
        }
    }
    catch (Exception& e)
    {
        log << "Exception at `" << e.file << "`: " << e.line << '\n';
        log << Debug::GetBacktraceString(e.callstack) << std::endl;
        DVASSERT(false && "check out log file");
        return false;
    }

    return true;
}

const DLCManager::IRequest* DLCManagerImpl::RequestPack(const String& packName)
{
    DVASSERT(Thread::IsMainThread());

    log << __FUNCTION__ << " packName: " << packName << std::endl;

    auto itPreloaded = preloadedPacks.find(packName);
    if (end(preloadedPacks) != itPreloaded)
    {
        const PreloadedPack& request = itPreloaded->second;
        return &request;
    }

    if (!IsInitialized())
    {
        PackRequest* request = AddDelayedRequest(packName);
        return request;
    }

    const PackRequest* request = FindRequest(packName);
    if (request == nullptr)
    {
        request = CreateNewRequest(packName);
    }
    return request;
}

void DLCManagerImpl::SetRequestPriority(const IRequest* request)
{
    DVASSERT(Thread::IsMainThread());

    if (request != nullptr)
    {
        const PackRequest* r = dynamic_cast<const PackRequest*>(request);

        log << __FUNCTION__ << " request: " << request->GetRequestedPackName() << std::endl;

        PackRequest* req = const_cast<PackRequest*>(r);
        if (IsInitialized())
        {
            requestManager->SetPriorityToRequest(req);
        }
        else
        {
            auto it = std::find(begin(delayedRequests), end(delayedRequests), request);
            if (it != end(delayedRequests))
            {
                delayedRequests.erase(it);
                delayedRequests.insert(delayedRequests.begin(), req);
            }
        }
    }
}

void DLCManagerImpl::RemovePack(const String& requestedPackName)
{
    DVASSERT(Thread::IsMainThread());

    // now we can work without CDN, so always wait for initialization is done
    if (!IsInitialized())
    {
        return;
    }

    PackRequest* request = FindRequest(requestedPackName);
    if (request != nullptr)
    {
        Vector<uint32> deps = request->GetDependencies();
        for (uint32 dependent : deps)
        {
            const String& depPackName = meta->GetPackInfo(dependent).packName;
            PackRequest* r = FindRequest(depPackName);
            if (nullptr != r)
            {
                String packToRemove = r->GetRequestedPackName();
                RemovePack(packToRemove);
            }
        }

        requestManager->Remove(request);

        auto it = find(begin(requests), end(requests), request);
        if (it != end(requests))
        {
            requests.erase(it);
        }

        delete request;
    }

    if (meta)
    {
        StringStream undeletedFiles;
        FileSystem* fs = GetEngineContext()->fileSystem;
        // remove all files for pack
        Vector<uint32> fileIndexes = meta->GetFileIndexes(requestedPackName);
        for (uint32 index : fileIndexes)
        {
            if (IsFileReady(index))
            {
                const String relFile = GetRelativeFilePath(index);

                FilePath filePath = dirToDownloadedPacks + (relFile + extDvpl);
                if (!fs->DeleteFile(filePath))
                {
                    if (fs->IsFile(filePath))
                    {
                        undeletedFiles << filePath.GetStringValue() << '\n';
                    }
                }
                scanFileReady[index] = false; // clear flag anyway
            }
        }
        String errMsg = undeletedFiles.str();
        if (!errMsg.empty())
        {
            Logger::Error("can't delete files: %s", errMsg.c_str());
        }
    }
}

DLCManager::Progress DLCManagerImpl::GetProgress() const
{
    using namespace DAVA;
    using namespace PackFormat;

    Progress progress;

    if (!IsInitialized())
    {
        lastProgress.isRequestingEnabled = false;
        return lastProgress;
    }

    progress.isRequestingEnabled = true;

    const Vector<PackFile::FilesTableBlock::FilesData::Data>& files = usedPackFile.filesTable.data.files;
    const size_t size = files.size();
    for (size_t indexOfFile = 0; indexOfFile < size; ++indexOfFile)
    {
        const auto& fileData = files[indexOfFile];
        uint64 fullFileSize = fileData.compressedSize + sizeof(LitePack::Footer);
        progress.total += fullFileSize;

        if (IsFileReady(indexOfFile))
        {
            progress.alreadyDownloaded += fullFileSize;
        }
        else
        {
            // is current file is downloading in requestManager queue?
            const PackMetaData::PackInfo& packInfo = meta->GetPackInfo(fileData.metaIndex);
            if (requestManager->IsInQueue(packInfo.packName))
            {
                progress.inQueue += fullFileSize;
            }
        }
    }

    lastProgress.total = progress.total;
    lastProgress.alreadyDownloaded = progress.alreadyDownloaded;
    lastProgress.inQueue = progress.inQueue;
    lastProgress.isRequestingEnabled = progress.isRequestingEnabled;

    return progress;
}

bool DLCManagerImpl::IsRequestingEnabled() const
{
    DVASSERT(Thread::IsMainThread());
    return isProcessingEnabled;
}

void DLCManagerImpl::SetRequestingEnabled(bool value)
{
    DVASSERT(Thread::IsMainThread());

    if (value)
    {
        if (!isProcessingEnabled)
        {
            isProcessingEnabled = true;
            if (requestManager)
            {
                requestManager->Start();
            }
        }
    }
    else
    {
        if (isProcessingEnabled)
        {
            isProcessingEnabled = false;
            if (requestManager)
            {
                requestManager->Stop();
            }
        }
    }
}

PackRequest* DLCManagerImpl::FindRequest(const String& requestedPackName) const
{
    DVASSERT(Thread::IsMainThread());

    for (auto request : requests)
    {
        if (request->GetRequestedPackName() == requestedPackName)
        {
            return request;
        }
    }

    for (auto request : delayedRequests)
    {
        if (request->GetRequestedPackName() == requestedPackName)
        {
            return request;
        }
    }

    return nullptr;
}

bool DLCManagerImpl::IsPackInQueue(const String& packName)
{
    DVASSERT(Thread::IsMainThread());
    if (!IsInitialized())
    {
        return false;
    }

    return requestManager->IsInQueue(packName);
}

const FilePath& DLCManagerImpl::GetLocalPacksDirectory() const
{
    DVASSERT(Thread::IsMainThread());
    return dirToDownloadedPacks;
}

const String& DLCManagerImpl::GetSuperPackUrl() const
{
    DVASSERT(Thread::IsMainThread());
    return urlToSuperPack;
}

String DLCManagerImpl::GetRelativeFilePath(uint32 fileIndex)
{
    uint32 startOfFilePath = startFileNameIndexesInUncompressedNames.at(fileIndex);
    return &uncompressedFileNames.at(startOfFilePath);
}

void DLCManagerImpl::StartScanDownloadedFiles()
{
    if (ScanState::Wait == scanState)
    {
        scanState = ScanState::Starting;
        scanThread = Thread::Create(MakeFunction(this, &DLCManagerImpl::ThreadScanFunc));
        scanThread->SetName("DLC::ThreadScanFunc");
        scanThread->Start();
    }
}

void DLCManagerImpl::RecursiveScan(const FilePath& baseDir, const FilePath& dir, Vector<LocalFileInfo>& files)
{
    ScopedPtr<FileList> fl(new FileList(dir, false));

    for (uint32 index = 0; index < fl->GetCount(); ++index)
    {
        const FilePath& path = fl->GetPathname(index);
        if (fl->IsNavigationDirectory(index))
        {
            continue;
        }
        if (fl->IsDirectory(index))
        {
            RecursiveScan(baseDir, path, files);
        }
        else
        {
            if (path.GetExtension() == extDvpl)
            {
                String fileName = path.GetAbsolutePathname();

                FILE* f = FileAPI::OpenFile(fileName, "rb");
                if (f == nullptr)
                {
                    Logger::Info("can't open file %s during scan", fileName.c_str());
                    continue;
                }

                bool needDeleteIncompleteFile = false;
                int32 footerSize = sizeof(PackFormat::LitePack::Footer);
                if (0 == fseek(f, -footerSize, SEEK_END))
                {
                    PackFormat::LitePack::Footer footer;
                    if (footerSize == fread(&footer, 1, footerSize, f))
                    {
                        LocalFileInfo info;
                        info.relativeName = path.GetRelativePathname(baseDir);
                        info.compressedSize = footer.sizeCompressed;
                        info.crc32Hash = footer.crc32Compressed;
                        files.push_back(info);
                    }
                    else
                    {
                        needDeleteIncompleteFile = true;
                        Logger::Info("can't read footer in file: %s", fileName.c_str());
                    }
                }
                else
                {
                    needDeleteIncompleteFile = true;
                    Logger::Info("can't seek to dvpl footer in file: %s", fileName.c_str());
                }
                FileAPI::Close(f);
                if (needDeleteIncompleteFile)
                {
                    if (0 != FileAPI::RemoveFile(fileName))
                    {
                        Logger::Error("can't delete incomplete file: %s", fileName.c_str());
                    }
                }
            }
        }
    }
}

void DLCManagerImpl::ScanFiles(const FilePath& dir, Vector<LocalFileInfo>& files)
{
    if (FileSystem::Instance()->IsDirectory(dir))
    {
        files.clear();
        files.reserve(hints.maxFilesToDownload);
        RecursiveScan(dir, dir, files);
    }
}

void DLCManagerImpl::ThreadScanFunc()
{
    Thread* thisThread = Thread::Current();
    // scan files in download dir
    int64 startTime = SystemTimer::GetMs();

    ScanFiles(dirToDownloadedPacks, localFiles);

    int64 finishScan = SystemTimer::GetMs() - startTime;

    Logger::Info("finish scan files for: %fsec total files: %ld", finishScan / 1000.f, localFiles.size());

    if (thisThread->IsCancelling())
    {
        return;
    }

    metaDataLoadedSem.Wait();

    if (thisThread->IsCancelling() || meta == nullptr)
    {
        return;
    }

    // merge with meta
    // Yes! is pack loaded before meta
    const PackFormat::PackFile& pack = GetPack();

    Vector<ResourceArchive::FileInfo> filesInfo;
    PackArchive::FillFilesInfo(pack, uncompressedFileNames, mapFileData, filesInfo);

    String relativeNameWithoutDvpl;

    if (thisThread->IsCancelling())
    {
        return;
    }

    FileSystem* fs = GetEngineContext()->fileSystem;

    for (const LocalFileInfo& info : localFiles)
    {
        relativeNameWithoutDvpl = info.relativeName.substr(0, info.relativeName.size() - 5);
        const PackFormat::FileTableEntry* entry = mapFileData[relativeNameWithoutDvpl];
        if (entry != nullptr)
        {
            if (entry->compressedCrc32 != info.crc32Hash || entry->compressedSize != info.compressedSize)
            {
                Logger::Info("hash not match for file: %s delete it", info.relativeName.c_str());
                fs->DeleteFile(dirToDownloadedPacks + info.relativeName);
            }
            else
            {
                size_t fileIndex = std::distance(&pack.filesTable.data.files[0], entry);
                scanFileReady[fileIndex] = true;
            }
        }
        else
        {
            // no such file on server, delete it
            fs->DeleteFile(dirToDownloadedPacks + info.relativeName);
        }
    }

    if (thisThread->IsCancelling())
    {
        return;
    }

    DAVA::RunOnMainThreadAsync([this]()
                               {
                                   // finish thread
                                   scanState = ScanState::Done;
                               });
}

} // end namespace DAVA
