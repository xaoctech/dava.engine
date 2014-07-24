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

#include "DataDownloadManager.h"

namespace DAVA
{


DataDownloadManager::CallbackData::CallbackData(uint32 _id, DownloadStatus _status)
    : id(_id)
    , status(_status)
{
    
}

Mutex DataDownloadManager::currentTaskMutex;

DataDownloadManager::DataDownloadManager()
    : thisThread(NULL)
    , isThreadStarted(false)
    , currentTask(NULL)
    , downloader(0)
    , callNotify(NULL)
{
}

DataDownloadManager::~DataDownloadManager()
{
    isThreadStarted = false;

    if (currentTask)
        Wait(downloader->InterruptDownload());

    ClearAll();

    SafeRelease(thisThread);
    SafeDelete(downloader);
}

void DataDownloadManager::SetDownloader(DataDownloader *_downloader)
{
    DVASSERT(true == Thread::IsMainThread());
    DVASSERT(NULL != _downloader);
    do
    {
        if (NULL == currentTask)
        {
            SafeDelete(downloader);
            downloader = _downloader;

            return;
        }
        Thread::SleepThread(20);
        Update();
    }while(1);
   
}

DataDownloader *DataDownloadManager::GetDownloader()
{
    return downloader;
}

void DataDownloadManager::SetNotificationCallback(DataDownloadManager::NotifyFunctor callbackFn)
{
    DVASSERT(true == Thread::IsMainThread());

    callNotify = callbackFn;
}

DataDownloadManager::NotifyFunctor DataDownloadManager::GetNotificationCallback() const
{
    DVASSERT(true == Thread::IsMainThread());

    return callNotify;
}


void DataDownloadManager::StartProcessingThread()
{
    // thread should be stopped
    DVASSERT(!isThreadStarted);
    DVASSERT(NULL == thisThread);

    thisThread = Thread::Create(Message(this, &DataDownloadManager::ThreadFunction));
    isThreadStarted = true;
    thisThread->Start();
}

void DataDownloadManager::StopProcessingThread()
{
    DVASSERT(true == Thread::IsMainThread());

    isThreadStarted = false;
    thisThread->Join();

    SafeRelease(thisThread);
}

void DataDownloadManager::Update()
{
    // use it only from Main thread
    DVASSERT(true == Thread::IsMainThread());

    bool isEmpty = true;
    do
    {
        CallbackData cbData(0, DL_UNKNOWN);

        callbackMutex.Lock();
        isEmpty = callbackMessagesQueue.empty();
        if(!isEmpty)
        {
            cbData = callbackMessagesQueue.front();
            callbackMessagesQueue.pop_front();
        }
        callbackMutex.Unlock();

        if(!isEmpty)
        {
           callNotify(cbData.id, cbData.status);
        }
    }
    while(!isEmpty);

    if (currentTask)
    {
        // if task is done and we have no pending tasks - stop processing thread.
        if (DL_FINISHED == currentTask->status)
        {
            PlaceToQueue(doneTaskQueue, currentTask);

            if (pendingTaskQueue.empty())
            {
                StopProcessingThread();

                currentTaskMutex.Lock();
                currentTask = NULL;
                currentTaskMutex.Unlock();
                return;
            }
        }
        else
            return;
    }

    if (pendingTaskQueue.empty())
        return;

    currentTaskMutex.Lock();
    currentTask = pendingTaskQueue.front();
    DVASSERT(0 < currentTask->id);
    pendingTaskQueue.pop_front();
    currentTaskMutex.Unlock();

    if (!isThreadStarted)
        StartProcessingThread();

    callbackMutex.Lock();
    if (!callbackMessagesQueue.empty())
    {
        for (Deque<CallbackData>::iterator it = callbackMessagesQueue.begin(); it != callbackMessagesQueue.end();)
        {
           CallbackData cbData = (*it);
           it = callbackMessagesQueue.erase(it);
           callNotify(cbData.id, cbData.status);
        }
    }
    callbackMutex.Unlock();
}

uint32 DataDownloadManager::GetSize(const String &srcUrl, uint32 timeout, uint32 retriesCount)
{
    DVASSERT(true == Thread::IsMainThread());

    DataDownloader::DownloadTaskDescription *task = new DataDownloader::DownloadTaskDescription(srcUrl, String(), GET_SIZE, timeout, retriesCount);
 
    task->id = CreateId();

    PlaceToQueue(pendingTaskQueue, task);

    SetTaskStatus(task, DL_PENDING);

    return task->id;
}

uint32 DataDownloadManager::Download(const String &srcUrl, const String &storeToFilePath, DownloadType downloadMode, uint32 timeout, uint32 retriesCount)
{
    DVASSERT(true == Thread::IsMainThread());

    DataDownloader::DownloadTaskDescription *task = new DataDownloader::DownloadTaskDescription(srcUrl, storeToFilePath, downloadMode, timeout, retriesCount);
 
    task->id = CreateId();

    PlaceToQueue(pendingTaskQueue, task);

    SetTaskStatus(task, DL_PENDING);

    return task->id;
}

uint32 DataDownloadManager::CreateId()
{
    static uint32 prevId = 1;
    return prevId++;
}

void DataDownloadManager::Retry(const uint32 &taskId)
{
    DVASSERT(true == Thread::IsMainThread());

    DataDownloader::DownloadTaskDescription *taskToRetry = ExtractFromQueue(doneTaskQueue, taskId);
    if (taskToRetry)
    {
        taskToRetry->error = DLE_NO_ERROR;
        taskToRetry->type = AUTO;
        SetTaskStatus(taskToRetry, DL_PENDING);
        PlaceToQueue(pendingTaskQueue, taskToRetry);
        Update();
    }
}

void DataDownloadManager::Cancel(const uint32 &taskId)
{
    DVASSERT(true == Thread::IsMainThread());

    DataDownloader::DownloadTaskDescription * curTaskToDelete = currentTask;

    if (curTaskToDelete && taskId == curTaskToDelete->id)
    {
       downloader->InterruptDownload();
       Wait(taskId);
    }
    else
    {
        DataDownloader::DownloadTaskDescription *pendingTask = NULL;
        pendingTask = ExtractFromQueue(pendingTaskQueue, taskId);
        if (pendingTask)
        {
            pendingTask->error = DLE_CANCELLED;
            SetTaskStatus(pendingTask, DL_FINISHED);
            PlaceToQueue(doneTaskQueue, pendingTask);
        }
    }
}

void DataDownloadManager::CancelCurrent()
{
    DVASSERT(true == Thread::IsMainThread());

    DataDownloader::DownloadTaskDescription * curTaskToCancel = currentTask;
    if (!curTaskToCancel)
        return;

    downloader->InterruptDownload();
    Wait(curTaskToCancel->id);
}

void DataDownloadManager::CancelAll()
{
    //queues changing allowed only from main thread
    DVASSERT(true == Thread::IsMainThread());

    if (!pendingTaskQueue.empty())
    {
        Deque<DataDownloader::DownloadTaskDescription *>::iterator it;
        for (it = pendingTaskQueue.begin(); it != pendingTaskQueue.end();)
        {
            DataDownloader::DownloadTaskDescription * task = (*it);
            task->error = DLE_CANCELLED;
            SetTaskStatus(task, DL_FINISHED);
            doneTaskQueue.push_back(task);
            it = pendingTaskQueue.erase(it);
        }
    }

    if (currentTask)
    {
        int32 id = downloader->InterruptDownload();
        Wait(id);
    }
}

void DataDownloadManager::Clear(const uint32 &taskId)
{
    //queues changing allowed only from main thread
    DVASSERT(true == Thread::IsMainThread());

    // cancel task if possible
    Cancel(taskId);

    DataDownloader::DownloadTaskDescription * task = NULL;
    task = ExtractFromQueue(pendingTaskQueue, taskId);
    if (task)
    {
        SafeDelete(task);
    }
    else
    {
        task = ExtractFromQueue(doneTaskQueue, taskId);
        if (task)
        {
            SafeDelete(task);
        }
    }
}

void DataDownloadManager::ThreadFunction(BaseObject *caller, void *callerData, void *userData)
{
    while(isThreadStarted)
    {
        Thread::SleepThread(20);

        currentTaskMutex.Lock();
        if (!currentTask || DL_FINISHED == currentTask->status)
        {
            currentTaskMutex.Unlock();
            continue;
        }

        DownloadError error = downloader->Download(currentTask);

        currentTask->error = error;
        currentTaskMutex.Unlock();
        
        // if we need to stop thread (finish current task end exit)
        if (!isThreadStarted)
            break;
    }
    currentTask = NULL;
    isThreadStarted = false;
}

void DataDownloadManager::ClearAll()
{
    ClearPending();
    ClearDone();

    DataDownloader::DownloadTaskDescription *currentTaskToClear = NULL;

    currentTaskToClear = currentTask;

    Clear(currentTaskToClear->id);
}

void DataDownloadManager::ClearPending()
{
    ClearQueue(pendingTaskQueue);
}

void DataDownloadManager::ClearDone()
{
    ClearQueue(doneTaskQueue);
}

void DataDownloadManager::Wait(const uint32 &taskId)
{
    DVASSERT(true == Thread::IsMainThread());

    // if you called it from other thread than main - you should be sured that Update() method calls periodically from Main Thread.

    DataDownloader::DownloadTaskDescription *waitTask = NULL;
    DataDownloader::DownloadTaskDescription *currentTaskToWait = NULL;

    currentTaskToWait = currentTask;

    if (currentTaskToWait && currentTaskToWait->id == taskId)
        waitTask = currentTaskToWait;

    if (!waitTask)
    {
        Deque<DataDownloader::DownloadTaskDescription *>::iterator it;
        for (it = pendingTaskQueue.begin(); it != pendingTaskQueue.end(); ++it)
        {
            DataDownloader::DownloadTaskDescription * task = (*it);
            if (taskId == task->id)
                waitTask = task;
        }
    }

    // if there is no such task or task is completed - we don;t need to wait
    if (NULL == waitTask || (DL_PENDING != waitTask->status && DL_IN_PROGRESS != waitTask->status))
        return;

    // wait until task is finished
    while (waitTask
       && (waitTask->status == DL_IN_PROGRESS || waitTask->status == DL_PENDING))
    {
        Thread::SleepThread(20);
        Update();
    }
}

void DataDownloadManager::WaitAll()
{
    DVASSERT(true == Thread::IsMainThread());
    while (true)
    {
        bool needToWait = currentTask || !pendingTaskQueue.empty();

        if (needToWait)
        {
            Thread::SleepThread(20);
            Update();
        }
        else
            break;
    }
}

bool DataDownloadManager::GetCurrentId(uint32 &id)
{
    DataDownloader::DownloadTaskDescription * curTaskToGet = currentTask;

    if (curTaskToGet)
    {
        id = curTaskToGet->id;
        return true;
    }

    return false;
}

bool DataDownloadManager::GetUrl(const uint32 &taskId, String &url)
{
    DataDownloader::DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    url = task->url;

    return true;
}

bool DataDownloadManager::GetType(const uint32 &taskId, DownloadType &type)
{
    DataDownloader::DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    type = task->type;

    return true;
}

bool DataDownloadManager::GetStatus(const uint32 &taskId, DownloadStatus &status)
{
    DataDownloader::DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    status = task->status;

    return true;
}

bool DataDownloadManager::GetTotal(const uint32 &taskId, uint64 &total)
{
    DataDownloader::DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    total = task->downloadTotal;

    return true;
}

bool DataDownloadManager::GetProgress(const uint32 &taskId, uint64 &progress)
{
    DataDownloader::DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    progress = task->downloadProgress;

    return true;
}

bool DataDownloadManager::GetError(const uint32 &taskId, DownloadError &error)
{
    DataDownloader::DownloadTaskDescription *task = GetTaskForId(taskId);
    if (!task)
        return false;

    error = task->error;

    return true;
}

uint64 DataDownloadManager::GetDownloadFileSize(const String &url)
{
    if (NULL == downloader)
        return -1;

    int64 size;
    downloader->GetDownloadFileSize(size, url);
    return size;
}

void DataDownloadManager::ClearQueue(Deque<DataDownloader::DownloadTaskDescription *> &queue)
{
    DVASSERT(true == Thread::IsMainThread());
    if (!queue.empty())
    {
        for (Deque<DataDownloader::DownloadTaskDescription *>::iterator it = queue.begin(); it != queue.end();)
        {
           DataDownloader::DownloadTaskDescription *task = (*it);
           delete task;
           it = queue.erase(it);
        }
    }
}

DataDownloader::DownloadTaskDescription *DataDownloadManager::ExtractFromQueue(Deque<DataDownloader::DownloadTaskDescription *> &queue, const uint32 &taskId)
{
    DataDownloader::DownloadTaskDescription *extractedTask = NULL;

    if (!queue.empty())
    {
        for (Deque<DataDownloader::DownloadTaskDescription *>::iterator it = queue.begin(); it != queue.end();)
        {
            DataDownloader::DownloadTaskDescription *task = (*it);
            if (task->id == taskId)
            {
                extractedTask = task;

                it = queue.erase(it);
            }
            else
                ++it;
        }
    }
    return extractedTask;
}

void DataDownloadManager::PlaceToQueue(Deque<DataDownloader::DownloadTaskDescription *> &queue, DataDownloader::DownloadTaskDescription *task)
{
    queue.push_back(task);
}

DataDownloader::DownloadTaskDescription *DataDownloadManager::GetTaskForId(const uint32 &taskId)
{
    DVASSERT(true == Thread::IsMainThread());

    DataDownloader::DownloadTaskDescription *retPointer = NULL;

    if (currentTask && taskId == currentTask->id)
    {
        retPointer = currentTask;
        return retPointer;
    }

    Deque<DataDownloader::DownloadTaskDescription *>::iterator it;
    for (it = pendingTaskQueue.begin(); it != pendingTaskQueue.end(); ++it)
    {
        retPointer = (*it);
        if (retPointer->id == taskId)
            return retPointer;
    }

    for (it = doneTaskQueue.begin(); it != doneTaskQueue.end(); ++it)
    {
        retPointer = (*it);
        if (retPointer->id == taskId)
            return retPointer;
    }

    return retPointer;
}

void DataDownloadManager::SetTaskStatus(DataDownloader::DownloadTaskDescription *task, const DownloadStatus &status)
{
    DVASSERT(task);
    DVASSERT(status != task->status);

    task->status = status;

    callbackMutex.Lock();
    callbackMessagesQueue.push_back(CallbackData(task->id, task->status));  
    callbackMutex.Unlock();
}

}
