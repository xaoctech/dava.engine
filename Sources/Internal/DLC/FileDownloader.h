#ifndef __DAVAENGINE_FILE_DOWNLOADER_H__
#define __DAVAENGINE_FILE_DOWNLOADER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class File;
typedef void CURL;


class FileDownloaderDelegate
{
public:
    enum DownloadStatusCode
    {
        DL_SUCCESS = 0, //
        DL_ERROR_FILE_SYSTEM,
        DL_ERROR_DOWNLOAD
    };
    
public:
    virtual ~FileDownloaderDelegate() = 0;
    
    virtual void DownloadGetPacket(uint64 size);
    virtual void DownloadReconnect();
    virtual void DownloadComplete(DownloadStatusCode status);
};

class FileDownloader: public BaseObject
{
public:
    //
    FileDownloader();
    FileDownloader(const String & _sourceUrl, const FilePath & _savePath, bool reload = false);
    virtual ~FileDownloader();
    
    // Start download file & save with same name
    // return type CURLcode
    virtual uint32 SynchDownload();
    virtual void AsynchDownload();
    
    virtual void Pause();
    virtual void Resume();
    virtual void Stop();
    
    // Getters & Setters
    virtual const String & GetSourceUrl() const;
    virtual void SetSourceUrl(const String & _sourceUrl);

    virtual const FilePath & GetSavePath() const;
    virtual void SetSavePath(const FilePath & _savePath);

    virtual uint16 GetMaxReconnect() const;
    virtual void  SetMaxReconnect(const uint16 _cnt);
    
    virtual FileDownloaderDelegate * GetDelegate() const;
    virtual void SetDelegate(FileDownloaderDelegate * _delegate);

    virtual bool GetReloadFile() const;
    virtual void SetReloadFile(const bool _reload);
    
protected:
    // Callback for cUrl write data
    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);

    // Method for DAVA::Message to create Thread
    void DownloadFile(BaseObject*, void*, void*);

    // Main download func 
    // return type CURLcode
    uint32 DownloadFile();
    
    // CURL download to tile
    // return type CURLcode
    uint32 CurlDownload();
    
    static bool isCURLInit;

    virtual CURL * GetCurlHandler() const;

    enum FdState
    {
        FD_RUN = 0,
        FD_PAUSE,
        FD_STOP
    };
    
private:
    String sourceUrl;
    FilePath savePath;
    uint16 reconnectCnt;
    int16 reconnectMax;
    bool reloadFile;
    
    FileDownloaderDelegate * delegate;
    File * fileForWrite;
    
    FdState state;
    bool isPauseChangeApplied;
    CURL *curl_handle;
};

}


#endif //__DAVAENGINE_FILE_DOWNLOADER_H__
