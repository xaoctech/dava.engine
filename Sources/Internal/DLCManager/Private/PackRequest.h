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
    PackRequest(DLCManagerImpl& packManager_, const String& packName, Vector<uint32_t> fileIndexes_);
    void CancelCurrentDownloadRequests();
    PackRequest(DLCManagerImpl& packManager_, const String& requestedPackName);

    ~PackRequest() override;

    void Start();
    bool Update();
    void Stop();

    const String& GetRequestedPackName() const override;
    /** recalculate full size with all dependencies */
    Vector<uint32_t> GetDependencies() const;
    /** return size of files within this request without dependencies */
    uint64 GetSize() const override;
    /** recalculate current downloaded size without dependencies */
    uint64 GetDownloadedSize() const override;

    /** return true when all files loaded and ready */
    bool IsDownloaded() const override;

    void SetFileIndexes(Vector<uint32_t> fileIndexes_);

    /** this request depends on other, so other should be downloaded first */
    bool IsSubRequest(const PackRequest* other) const;

    bool IsDelayed() const;

    PackRequest& operator=(PackRequest&& other);

private:
    void InitializeFileRequests();

    enum Status : uint32_t
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
        FilePath localFile;
        String errorMsg;
        String url;
        uint32_t fileIndex = 0;
        uint32_t hashFromMeta = 0;
        uint64_t startLoadingPos = 0;
        uint64_t sizeOfCompressedFile = 0;
        uint64_t sizeOfUncompressedFile = 0;
        uint64_t downloadedFileSize = 0;
        DLCDownloader::Task* task = nullptr;
        Compressor::Type compressionType = Compressor::Type::Lz4HC;
        Status status = Wait;
    };

    void InitializeFileRequest(const uint32_t fileIndex,
                               const FilePath& file,
                               const uint32_t hash,
                               const uint64_t startLoadingPos,
                               const uint64_t fileCompressedSize,
                               const uint64_t fileUncompressedSize,
                               const String& url,
                               const Compressor::Type compressionType_,
                               FileRequest& fileRequest);

    static void DeleteJustDownloadedFileAndStartAgain(FileRequest& fileRequest);
    void DisableRequestingAndFireSignalIOError(FileRequest& fileRequest, int32_t errVal) const;
    bool CheckLocalFileState(FileSystem* fs, FileRequest& fileRequest);
    bool CheckLoadingStatusOfFileRequest(FileRequest& fileRequest, DLCDownloader* dm, const String& dstPath);
    bool LoadingPackFileState(FileSystem* fs, FileRequest& fileRequest);
    bool CheckHaskState(FileRequest& fileRequest);
    bool UpdateFileRequests();

    DLCManagerImpl* packManagerImpl = nullptr;

    Vector<FileRequest> requests;
    Vector<uint32_t> fileIndexes;
    String requestedPackName;
    mutable Vector<uint32_t> dependencyCache;

    uint32_t numOfDownloadedFile = 0;

    // if this field is false, you can check fileIndexes
    // else fileIndexes maybe empty and wait initialization
    bool delayedRequest = true;
};

inline bool PackRequest::IsDelayed() const
{
    return delayedRequest;
}

} // end namespace DAVA
