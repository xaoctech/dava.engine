/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __DATA_DOWNLOADER_H__
#define __DATA_DOWNLOADER_H__

#include "Base/BaseTypes.h"
#include "Platform/Thread.h"
#include "Platform/Mutex.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{

enum DownloadType
{
    AUTO = 0,
    FULL,
    RESUMED,
    GET_SIZE,
};

enum DownloadStatus
{
    DL_PENDING = 0,
    DL_IN_PROGRESS,
    DL_FINISHED,
    DL_UNKNOWN,
};
        
enum DownloadError
{
    DLE_NO_ERROR = 0,
    DLE_CANCELLED,
    DLE_CANNOT_RESUME,
    DLE_CANNOT_CONNECT,
    DLE_CONTENT_NOT_FOUND,
    DLE_COMMON_ERROR,
    DLE_INIT_ERROR,        
    DLE_FILE_ERROR,
    DLE_UNKNOWN,
};

class DataDownloader
{
    friend class DataDownloadManager;

    struct DownloadTaskDescription
    {
        DownloadTaskDescription(const String &srcUrl, const String &storeToFilePath, DownloadType downloadMode, uint32 _timeout, uint32 _retriesCount);

        uint32 id;
        String url;
        String storePath;
        uint32 timeout;
        uint32 retriesCount;
        uint32 retriesLeft;
        DownloadType type;
        DownloadStatus status;
        DownloadError error;
    	int64 downloadTotal;
    	int64 downloadProgress;
    };

public:
    DataDownloader(uint32 operationTimeout = 3000, uint8 operationRetryesAllowed = 20);

protected:
    /* Main download operation. Covers and calls platform depended download.*/
    DownloadError Download(DownloadTaskDescription *taskDescr);
    virtual uint32 InterruptDownload();
    virtual DownloadError GetDownloadFileSize(int64 &retSize, const String &url = "") = 0;
    virtual DownloadError DownloadContent(const uint64 &loadFrom) = 0;
    DownloadError TryDownload();

    virtual size_t SaveData(void *ptr, size_t size, size_t nmemb);

    void ResetretriesCount();
    bool IsInterrupting();

protected:  
    DownloadTaskDescription *currentTask;
    uint8 retriesLeft;
    const uint32 timeout;
    uint8 downloadProgress;

private:
    bool isDownloadInterrupting;
    const uint8 retriesCount;
    static Mutex singleDownloadMutex;
    int64 remoteFileSize;
    uint64 downloadedTotal;
};

}

#endif