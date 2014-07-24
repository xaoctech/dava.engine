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

#ifndef __DATA_DOWNLOAD_MANAGER_H__
#define __DATA_DOWNLOAD_MANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Base/Function.h"
#include "DLC/Downloader/DataDownloader.h"

namespace DAVA
{

class DataDownloadManager : public Singleton<DataDownloadManager>
{
    friend class DataDownloader;

    struct CallbackData
    {
        CallbackData(uint32 _id, DownloadStatus _status);

        uint32 id;
        DownloadStatus status;
    };

public:
    typedef Function<void (const uint32 &, const DownloadStatus &)> NotifyFunctor;

public:
    DataDownloadManager();
    virtual ~DataDownloadManager();
    void SetDownloader(DataDownloader *_downloader);
    DataDownloader *GetDownloader();
    void SetNotificationCallback(NotifyFunctor callbackFn);
    NotifyFunctor GetNotificationCallback() const;

    void Kill();
    void Update();

    uint32 GetSize(const String &srcUrl, uint32 timeout, uint32 retriesCount);
    uint32 Download(const String &srcUrl, const String &storeToFilePath, DownloadType downloadMode = AUTO, uint32 timeout = 2000, uint32 retriesCount = -1);
    uint32 CreateId();
    void Retry(const uint32 &taskId);

    void CancelCurrent();
    void Cancel(const uint32 &taskId);
    void CancelAll();

    void Wait(const uint32 &taskId);
    void WaitAll();

    bool GetCurrentId(uint32 &id);
    bool GetUrl(const uint32 &taskId, String &url);
    bool GetType(const uint32 &taskId, DownloadType &type);
    bool GetStatus(const uint32 &taskId, DownloadStatus &status);
    bool GetTotal(const uint32 &taskId, uint64 &total);
    bool GetProgress(const uint32 &taskId, uint64 &progress);
    bool GetError(const uint32 &taskId, DownloadError &error);
    uint64 GetDownloadFileSize(const String &url);


protected:
    void SetTaskStatus(DataDownloader::DownloadTaskDescription *task, const DownloadStatus &status);

private:
    void StartProcessingThread();
    void StopProcessingThread();
    void ThreadFunction(BaseObject *caller, void *callerData, void *userData);
    
    void ClearQueue(Deque<DataDownloader::DownloadTaskDescription *> &queue);
    DataDownloader::DownloadTaskDescription *ExtractFromQueue(Deque<DataDownloader::DownloadTaskDescription *> &queue, const uint32 &taskId);
    void PlaceToQueue(Deque<DataDownloader::DownloadTaskDescription *> &queue, DataDownloader::DownloadTaskDescription *task);

    DataDownloader::DownloadTaskDescription *GetTaskForId(const uint32 &taskId);

    void Clear(const uint32 &taskId);
    void ClearAll(); 
    void ClearPending();
    void ClearDone();

private:
    Thread *thisThread;

    bool isThreadStarted;
    Deque<DataDownloader::DownloadTaskDescription *> pendingTaskQueue;
    Deque<DataDownloader::DownloadTaskDescription *> doneTaskQueue;
    Deque<CallbackData> callbackMessagesQueue;
    DataDownloader::DownloadTaskDescription *currentTask;
    static Mutex currentTaskMutex;
    Mutex callbackMutex;
    DataDownloader *downloader;
    NotifyFunctor callNotify;
};

}

#endif