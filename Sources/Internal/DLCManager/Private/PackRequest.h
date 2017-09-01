#pragma once

#include "DLCManager/DLCDownloader.h"
#include "DLCManager/DLCManager.h"
#include "FileSystem/FilePath.h"
#include "Compression/Compressor.h"
#include "Utils/CRC32.h"

#include <fstream>

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

    // clear redundant data to free memory
    void Finalize();

    void SetFileIndexes(Vector<uint32> fileIndexes_);

    /** this request depends on other, so other should be downloaded first */
    bool IsSubRequest(const PackRequest* other) const;

private:
    void InitializeFileRequests();

    enum Status : uint32
    {
        CheckLocalFile,
        LoadingPackFile, // download manager thread, wait on main thread
        Ready, // on main thread

        Error
    };

    /**
	   DVPLWriter - is class for optimization only. I try to do:
	   1 - downloading of data
	   2 - crc32 hash check
	   3 - write footer
	   all inside downloader thread.
	   So DVPLWriter doing 2 task:
	   1 - writing data to disk
	   2 - checking data, moving files, and adding footer
	*/
    class DVPLWriter : public DLCDownloader::IWriter
    {
    public:
        DVPLWriter(FilePath& localPath_, uint32 sizeCompressed_, uint32 sizeUncompressed_, uint32 crc32Compressed_, Compressor::Type compressionType_)
            : localPath(localPath_)
            , sizeCompressed(sizeCompressed_)
            , sizeUncompressed(sizeUncompressed_)
            , crc32Compressed(crc32Compressed_)
            , compressionType(compressionType_)
        {
        }
        bool OpenFile();
        /** Save next buffer bytes into memory or file, on error return differs from parameter size */
        uint64 Save(const void* ptr, uint64 size) override;
        /** Return current size of saved byte stream, return ```std::numeric_limits<uint64>::max()``` value on error */
        uint64 GetSeekPos() override;
        /** Truncate file(or buffer) to zero length, return false on error */
        bool Truncate() override;
        /** Close internal resource (file handle, socket, free memory) */
        bool Close() override;
        /** Check internal state */
        bool IsClosed() const override;

    private:
        std::ofstream fout;
        CRC32 crc32counter;
        FilePath localPath;
        const uint32 sizeCompressed;
        const uint32 sizeUncompressed;
        const uint32 crc32Compressed;
        const Compressor::Type compressionType;
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
        Status status = CheckLocalFile;
        std::shared_ptr<DVPLWriter> dvplWriter;
    };

    void DisableRequestingAndFireSignalIOError(FileRequest& fileRequest, int32 errVal, const String& extMsg) const;
    bool CheckLocalFileState(FileSystem* fs, FileRequest& fileRequest);
    bool CheckLoadingStatusOfFileRequest(FileRequest& fileRequest, DLCDownloader& dm, const String& dstPath);
    bool LoadingPackFileState(FileSystem* fs, FileRequest& fileRequest);
    bool UpdateFileRequests();

    DLCManagerImpl* packManager = nullptr;

    Vector<FileRequest> requests;
    Vector<uint32> fileIndexes;
    String requestedPackName;
    mutable Vector<uint32> dependencyCache;

    uint64 totalDownloadedSize = 0;

    // if this field is false, you can check fileIndexes
    // else fileIndexes maybe empty and wait initialization
    bool delayedRequest = true;
    bool fileRequestsInitialized = false;
    bool mutable isDownloaded = false;
};

} // end namespace DAVA
