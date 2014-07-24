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

#include "DataDownloader.h"
#include "DataDownloadManager.h"

namespace DAVA
{

DataDownloader::DownloadTaskDescription::DownloadTaskDescription(const String &srcUrl, const String &storeToFilePath, DownloadType downloadMode, uint32 _timeout, uint32 _retriesCount)
    : id(0)
    , url(srcUrl)
    , storePath(storeToFilePath)
    , timeout(_timeout)
    , retriesCount(_retriesCount)
    , retriesLeft(retriesCount)
    , type(downloadMode)
    , status(DL_UNKNOWN)
    , error(DLE_NO_ERROR)
    , downloadTotal(0)
    , downloadProgress(0)
{

}

Mutex DataDownloader::singleDownloadMutex;


DataDownloader::DataDownloader(uint32 operationTimeout, uint8 operationRetryesAllowed)
    : currentTask(NULL)
    , retriesLeft(operationRetryesAllowed)
    , timeout(operationTimeout)
    , downloadProgress(0)
    , isDownloadInterrupting(false)
    , retriesCount(operationRetryesAllowed)
    , remoteFileSize(0)
    , downloadedTotal(0)
{

}

uint32 DataDownloader::InterruptDownload()
{
    DVASSERT(currentTask);
    isDownloadInterrupting = true;
    return currentTask->id;
}

DownloadError DataDownloader::Download(DownloadTaskDescription *taskDescr)
{
    singleDownloadMutex.Lock();

    currentTask = taskDescr;

    FilePath path(currentTask->storePath);
    FileSystem::Instance()->CreateDirectory(path.GetDirectory(), true);

    if (AUTO == taskDescr->type)
    {// Try to determine download mode automatically
    
        // Check file exist
        File *fileCheckExistence = File::Create(path, File::OPEN | File::READ);
        if (NULL == fileCheckExistence)
            currentTask->type = FULL;
        else
        {    
            SafeRelease(fileCheckExistence);

            // we suppose that server supports download resuming. If not - we'll get download result.
            currentTask->type = RESUMED;
        }
    }

    ResetretriesCount();
    DownloadError error = DLE_NO_ERROR;
    isDownloadInterrupting = false;
    DataDownloadManager::Instance()->SetTaskStatus(currentTask, DL_IN_PROGRESS);
    downloadedTotal = 0;

    do
    {
        error = TryDownload();

        if (DLE_CANCELLED == currentTask->error)
            break;

        // server doesn't supports download resuming. So we need to download whole file.
        if (DLE_CANNOT_RESUME == currentTask->error)
            currentTask->type = FULL;

    }while (0 < --currentTask->retriesLeft && DLE_NO_ERROR != currentTask->error);

    if (DLE_NO_ERROR != currentTask->error && 0 >= currentTask->retriesLeft) // we cannot reconnect to server
        error = DLE_CANNOT_CONNECT; 

    singleDownloadMutex.Unlock();

    return error;
}

DownloadError DataDownloader::TryDownload()
{

    // retrieve remote file size
    DownloadError error = GetDownloadFileSize(remoteFileSize);
    if (DLE_NO_ERROR != error)
    {
        DataDownloadManager::Instance()->SetTaskStatus(currentTask, DL_FINISHED);
        return error;
    }

    if (GET_SIZE == currentTask->type)
    {
        currentTask->downloadTotal = remoteFileSize;
        currentTask->error = error;
        DataDownloadManager::Instance()->SetTaskStatus(currentTask, DL_FINISHED);
        return error;
    }

    DVASSERT(GET_SIZE != currentTask->type);

    int64 loadFrom = 0;

    // get downloaded part of file size
    if (RESUMED == currentTask->type)
    {
        // if file is particulary downloaded, wi will try to download rest part of it        
        File *fileToGetSize = File::Create(currentTask->storePath, File::OPEN | File::READ);
        if (!fileToGetSize)
        {
            DataDownloadManager::Instance()->SetTaskStatus(currentTask, DL_FINISHED);
            return DLE_FILE_ERROR;
        }

        loadFrom = fileToGetSize->GetSize();
        currentTask->downloadProgress = loadFrom;
        SafeRelease(fileToGetSize);
    }



    currentTask->downloadTotal = remoteFileSize;
    
    downloadedTotal = loadFrom;

    // if downloaded part of file is larger or equals to expected size
    if (loadFrom > remoteFileSize)
    {        
        // here we can interrupt download or reload file for example
        currentTask->type = FULL;
        loadFrom = 0;
    }
    else if (loadFrom == remoteFileSize)
    {
        DataDownloadManager::Instance()->SetTaskStatus(currentTask, DL_FINISHED);
        return DLE_NO_ERROR;
    }

    error = DownloadContent(loadFrom);

    if (DLE_NO_ERROR == error && isDownloadInterrupting)
    {
        isDownloadInterrupting = false;
        currentTask->error = DLE_CANCELLED;
        DataDownloadManager::Instance()->SetTaskStatus(currentTask, DL_FINISHED);
    }
    else
    {
        DataDownloadManager::Instance()->SetTaskStatus(currentTask, DL_FINISHED);
    }

    return error;
}


size_t DataDownloader::SaveData(void *ptr, size_t size, size_t nmemb)
{
    File *destFile;
    switch(currentTask->type)
    {
    case FULL:    // If file not exist create new file
        {
            destFile = File::Create(currentTask->storePath, File::CREATE | File::WRITE);
            currentTask->type = RESUMED;
        }break;

    case RESUMED: // Open file for APPEND and get pos to resume download
        {
            destFile = File::Create(currentTask->storePath, File::APPEND | File::WRITE);
        }break;

    default:
        DVASSERT(false);
        break;
    }

    uint32 written = destFile->Write(ptr, size * nmemb);

    downloadedTotal += written;
    currentTask->downloadProgress = downloadedTotal;

    SafeRelease(destFile);

    ResetretriesCount();

    if (isDownloadInterrupting)
        written = 0;

    return written;
}

void DataDownloader::ResetretriesCount()
{
   currentTask->retriesLeft = currentTask->retriesCount;
}

bool DataDownloader::IsInterrupting()
{
    return isDownloadInterrupting;
}

}