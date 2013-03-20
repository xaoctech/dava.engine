//
//  FileDownloader.cpp
//  WoTSniperMacOS
//
//  Created by Andrey Panasyuk on 3/21/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "FileDownloader.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "Platform/Thread.h"

#include "curl/curl.h"

namespace DAVA
{

    
bool FileDownloader::isCURLInit = false;

FileDownloader::FileDownloader()
    : BaseObject()
    , sourceUrl("")
    , savePath("")
    , reconnectCnt(0)
    , reconnectMax(1)
    , reloadFile(false)
    , delegate(NULL)
    , fileForWrite(NULL)
    , state(FD_RUN)
    , curl_handle(NULL)
    , isPauseChangeApplied(true)
{
    // Global init CURL
    if ( !FileDownloader::isCURLInit && curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK )
    {
        FileDownloader::isCURLInit = true;
    }
}
FileDownloader::FileDownloader(const FilePath & _sourceUrl, const FilePath & _savePath, bool reload)
    : BaseObject()
    , sourceUrl(_sourceUrl)
    , savePath(_savePath)
    , reconnectCnt(0)
    , reconnectMax(1)
    , reloadFile(reload)
    , delegate(NULL)
    , fileForWrite(NULL)
    , state(FD_RUN)
    , curl_handle(NULL)
    , isPauseChangeApplied(true)
{
    // Global init CURL
    if ( !FileDownloader::isCURLInit && curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK )
    {
        FileDownloader::isCURLInit = true;
    }
}

FileDownloader::~FileDownloader()
{
    
}

uint32 FileDownloader::SynchDownload()
{
    return DownloadFile();
}

void FileDownloader::AsynchDownload()
{
    Thread * downloadThread = Thread::Create( Message( this, &FileDownloader::DownloadFile ) );
    downloadThread->Start();
    downloadThread->Release();
}


size_t FileDownloader::write_data(void *ptr, size_t size, size_t nmemb, void *fileDownloader)
{
    FileDownloader * fd = (FileDownloader*)fileDownloader;
    File * destFile = fd->fileForWrite;
    
    int written = destFile->Write(ptr, size * nmemb);
    
    if ( fd->delegate != NULL )
    {
        fd->delegate->DownloadGetPacket(written);
    }
    
    // Pause & resume download 
    if ( fd->state == FD_PAUSE && !fd->isPauseChangeApplied )
    {
        curl_easy_pause( fd->GetCurlHandler(), CURLPAUSE_ALL );
        fd->isPauseChangeApplied = true;
        Logger::Instance()->Debug(" ----- Downloader pause");
    }
    
    // Stop download
    if ( fd->state == FD_STOP )
    {
        return CURL_READFUNC_ABORT;
    }
    
    return written;
}
    
void FileDownloader::DownloadFile(BaseObject* bo, void* v1, void* v2)
{
    DownloadFile();
}
    
uint32 FileDownloader::DownloadFile()
{
    Logger::Instance()->Debug("Start download");

    fileForWrite = NULL;
    
    FilePath path = GetSourceUrl().GetDirectory();
    String fileName = GetSourceUrl().GetFilename();
    
    FilePath fullSavePath = GetSavePath() + FilePath("/" + fileName);
    
    // Check file exist
    fileForWrite = File::Create(fullSavePath, File::OPEN | File::READ);
    if (fileForWrite == NULL)
    {
        // If fole not exist create new file
        fileForWrite = File::Create(fullSavePath, File::CREATE | File::WRITE);
    }
    else if (reloadFile == false)
    {
        // Open file for APPEND and get pos to resume download
        SafeRelease(fileForWrite);
        fileForWrite = File::Create(fullSavePath, File::APPEND | File::WRITE);
    }
    else
    {
        // Reload exist file from server
        SafeRelease(fileForWrite);
        fileForWrite = File::Create(fullSavePath, File::CREATE | File::WRITE);
    }
    
    // Error read, write or create file
    if ( fileForWrite == NULL && delegate != NULL)
    {
        delegate->DownloadComplete(FileDownloaderDelegate::DL_ERROR_FILE_SYSTEM);
        return CURL_LAST;
    }
    
    uint32 status;
    do
    {
        if (reconnectCnt > 0 && delegate != NULL)
        {
            delegate->DownloadReconnect();
        }
        status = CurlDownload();
        ++reconnectCnt;
    }
    while ( status != CURLE_OK && ( reconnectCnt < reconnectMax || reconnectMax == -1 ) );
    SafeRelease(fileForWrite);
    
    //
    if ( status == CURLE_OK && delegate != NULL )
    {
        delegate->DownloadComplete(FileDownloaderDelegate::DL_SUCCESS);
    }
    else if ( delegate != NULL )
    {
        delegate->DownloadComplete(FileDownloaderDelegate::DL_ERROR_DOWNLOAD);
    }

    Logger::Instance()->Debug("End download");
    
    return status;
}

uint32 FileDownloader::CurlDownload()
{
    uint64 loadFrom = fileForWrite->GetPos();

    /* init the curl session */ 
    curl_handle = curl_easy_init();
    
    /* set URL to get */ 
    curl_easy_setopt(curl_handle, CURLOPT_URL, GetSourceUrl().ResolvePathname().c_str());
    
    /* no progress meter please */ 
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    
    /* send all data to this function  */ 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, FileDownloader::write_data);
    
    // resume download from last position
    curl_easy_setopt(curl_handle, CURLOPT_RESUME_FROM, loadFrom);
    
    
    /* we want the body to this file handle */ 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, this);
    
    /*
     * Notice here that if you want the actual data sent anywhere else but
     * stdout, you should consider using the CURLOPT_WRITEDATA option.  */ 
    
    /* get it! */ 
    CURLcode status = curl_easy_perform(curl_handle);
    
    /* cleanup curl stuff */ 
    curl_easy_cleanup(curl_handle);   
    
    return status;
}

void FileDownloader::Pause()
{
    if ( state != FD_PAUSE )
    {
        return;
    }
    
    state = FD_PAUSE;
    isPauseChangeApplied = false;
}

void FileDownloader::Resume()
{
    if ( state == FD_PAUSE )
    {
        return;
    }
    
    state = FD_RUN;
    isPauseChangeApplied = true;
    curl_easy_pause( GetCurlHandler(), CURLPAUSE_CONT );
    Logger::Instance()->Debug(" ----- Downloader resume");
}
    
void FileDownloader::Stop()
{
    state = FD_STOP;
}

CURL * FileDownloader::GetCurlHandler() const
{
    return curl_handle;
}

const FilePath & FileDownloader::GetSourceUrl() const
{
    return sourceUrl;
}

void FileDownloader::SetSourceUrl(const FilePath & _sourceUrl)
{
    sourceUrl = _sourceUrl;
}
    
const FilePath & FileDownloader::GetSavePath() const
{
    return savePath;
}
    
void FileDownloader::SetSavePath(const FilePath & _savePath)
{
    savePath = _savePath;
}
    
uint16 FileDownloader::GetMaxReconnect() const
{
    return reconnectMax;
}
    
void  FileDownloader::SetMaxReconnect(const uint16 _cnt)
{
    reconnectMax = _cnt;
}

FileDownloaderDelegate * FileDownloader::GetDelegate() const
{
    return delegate;
}
    
void FileDownloader::SetDelegate(FileDownloaderDelegate * _delegate)
{
    delegate = _delegate;
}

bool FileDownloader::GetReloadFile() const
{
    return reloadFile;
}

void  FileDownloader::SetReloadFile(const bool _reload)
{
    reloadFile = _reload;
}

//
FileDownloaderDelegate::~FileDownloaderDelegate(){}
    
    
}//END DAVA








