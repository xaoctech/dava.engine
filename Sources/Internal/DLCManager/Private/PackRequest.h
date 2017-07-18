#pragma once

#include "DLCManager/DLCDownloader.h"
#include "DLCManager/DLCManager.h"
#include "FileSystem/FilePath.h"
#include "Compression/Compressor.h"

namespace DAVA
{
class DLCManagerImpl;
class FileSystem;

/**
	Download several files with one request
*/
class PackRequest : public DLCManager::IRequest
{
public:
    PackRequest(DLCManagerImpl& packManager_, const String& packName, Vector<uint32> fileIndexes_);
    void CancelCurrentDownloadRequests();
    PackRequest(DLCManagerImpl& packManager_, const String& requestedPackName);

    ~PackRequest() override;

    void Start();
    bool Update();
    void Stop();

    const String& GetRequestedPackName() const override;
    /** recalculate full size with all dependencies */
    Vector<uint32> GetDependencies() const;
    /** return size of files within this request without dependencies */
    uint64 GetSize() const override;
    /** recalculate current downloaded size without dependencies */
    uint64 GetDownloadedSize() const override;

    /** return true when all files loaded and ready */
    bool IsDownloaded() const override;

    void SetFileIndexes(Vector<uint32> fileIndexes_);

    /** this request depends on other, so other should be downloaded first */
    bool IsSubRequest(const PackRequest* other) const;

private:
    void InitializeFileRequests();

    enum Status : uint32
    {
        Wait = 0,
        CheckLocalFile,
        LoadingPackFile, // download manager thread, wait on main thread
        CheckHash, // on main thread (in future move to job manager)
        Ready, // on main thread

        Error
    };

    struct FileRequest
    {
        FileRequest() = default;
        FileRequest(FilePath localFile_,
                    String url_,
                    uint32 fileIndex_,
                    uint32 compressedCrc32_,
                    uint64 startLoadingPos_,
                    uint64 sizeOfCompressedFile_,
                    uint64 sizeOfUncompressedFile_,
                    DLCDownloader::Task* task_,
                    Compressor::Type compressionType_,
                    Status status_);
        ~FileRequest();

        FilePath localFile;
        String url;
        uint32 fileIndex = 0;
        uint32 compressedCrc32 = 0;
        uint64 startLoadingPos = 0;
        uint64 sizeOfCompressedFile = 0;
        uint64 sizeOfUncompressedFile = 0;
        uint64 downloadedFileSize = 0;
        DLCDownloader::Task* task = nullptr;
        Compressor::Type compressionType = Compressor::Type::Lz4HC;
        Status status = Wait;
    };

    static void DeleteJustDownloadedFileAndStartAgain(FileRequest& fileRequest);
    void DisableRequestingAndFireSignalIOError(FileRequest& fileRequest, int32 errVal, const String& extMsg) const;
    bool CheckLocalFileState(FileSystem* fs, FileRequest& fileRequest);
    bool CheckLoadingStatusOfFileRequest(FileRequest& fileRequest, DLCDownloader& dm, const String& dstPath);
    bool LoadingPackFileState(FileSystem* fs, FileRequest& fileRequest);
    bool CheckHaskState(FileRequest& fileRequest);
    bool UpdateFileRequests();

    DLCManagerImpl* packManagerImpl = nullptr;

    Vector<FileRequest> requests;
    Vector<uint32> fileIndexes;
    String requestedPackName;
    mutable Vector<uint32> dependencyCache;

    uint32 numOfDownloadedFile = 0;

    int32 openRetryCounter = 10; // 10 frames to try write append footer

    // if this field is false, you can check fileIndexes
    // else fileIndexes maybe empty and wait initialization
    bool delayedRequest = true;
    bool fileRequestsInitialized = false;
};

} // end namespace DAVA
