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

#include "DLC.h"
#include "Job/JobManager.h"
#include "Patcher/PatchFile.h"
#include "Downloader/DataDownloadManager.h"
#include "Platform/DeviceInfo.h"
#include "Render/GPUFamilyDescriptor.h"

namespace DAVA
{

DLC::DLC(const String &url, const FilePath &sourceDir, const FilePath &destinationDir, const FilePath &workingDir, uint32 gVersion, bool forceFullUpdate)
: dlcState(DS_INIT)
, dlcError(DE_NO_ERROR)
, patchingThread(NULL)
{
    DVASSERT(workingDir.IsDirectoryPathname());
    DVASSERT(workingDir.Exists());

    DVASSERT(destinationDir.IsDirectoryPathname());
    DVASSERT(destinationDir.Exists());

    DVASSERT(sourceDir.IsDirectoryPathname());
    DVASSERT(sourceDir.Exists());

    // initial values
    dlcContext.remoteUrl = url;
    dlcContext.localVer = 0;
    dlcContext.forceFullUpdate = forceFullUpdate;

    dlcContext.localWorkingDir = workingDir;
    dlcContext.localDestinationDir = destinationDir;
    dlcContext.localSourceDir = sourceDir;
    dlcContext.localVerStorePath = destinationDir + "Version.txt";

    dlcContext.remoteVerUrl = Format("%s/g%u.info", url.c_str(), gVersion);
    dlcContext.remoteVer = 0;
    dlcContext.remoteVerDownloadId = 0;
    dlcContext.remoteFullSizeDownloadId = 0;
    dlcContext.remoteLiteSizeDownloadId = 0;
    dlcContext.remoteVerStotePath = workingDir + "RemoteVersion.info";
    dlcContext.remotePatchDownloadId = 0;
    dlcContext.remotePatchSize = 0;
    dlcContext.remotePatchStorePath = workingDir + "Remote.patch";

    dlcContext.patchingOk = true;
    dlcContext.patchCount = 0;
    dlcContext.patchIndex = 0;

    dlcContext.downloadInfoStorePath = workingDir + "Download.info";
    dlcContext.stateInfoStorePath = workingDir + "State.info";
    dlcContext.prevState = ReadUint32(dlcContext.stateInfoStorePath);

    // FSM variables
    fsmAutoReady = false;
}

DLC::~DLC()
{
}

void DLC::Check()
{
    FSM(EVENT_CHECK_START);
}

void DLC::Start()
{ 
    FSM(EVENT_DOWNLOAD_START);
}

void DLC::Cancel()
{
    FSM(EVENT_CANCEL);
}

void DLC::GetProgress(uint64 &cur, uint64 &total) const
{
    switch(dlcState)
    {
        case DS_READY:
        case DS_DOWNLOADING:
            total = dlcContext.remotePatchSize;
            DataDownloadManager::Instance()->GetProgress(dlcContext.remotePatchDownloadId, cur);
            break;
        case DS_PATCHING:
            total = dlcContext.patchCount;
            cur = dlcContext.patchIndex;
            break;
        default:
            cur = 0;
            total = 0;
            break;
    }
}

DLC::DLCState DLC::GetState() const
{
    return dlcState;
}

DLC::DLCError DLC::GetError() const
{
    return dlcError;
}

void DLC::FSM(int event)
{
    bool eventHandled = true;
    DLCState oldState = dlcState;

    // State Machine implementation 
    // Read more - http://www.ict.edu.ru/ft/001845/switch.pdf

    switch(dlcState)
    {
        case DS_INIT:
            switch(event)
            {
                case EVENT_DOWNLOAD_START:
                    fsmAutoReady = true;
                    dlcState = DS_CHECKING_INFO;
                    break;
                case EVENT_CHECK_START:
                    dlcState = DS_CHECKING_INFO;
                    break;
                case EVENT_CANCEL:
                    dlcError = DE_WAS_CANCELED;
                    dlcState = DS_DONE;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;

        case DS_CHECKING_INFO:
            switch(event)
            {
                case EVENT_CHECK_OK:
                    // if remote version is newer than local we should continue DLC
                    if(dlcContext.localVer < dlcContext.remoteVer)
                    {
                        // check if patch exists on server
                        dlcState = DS_CHECKING_PATCH;
                    }
                    else
                    {
                        // local version is up do date. finish DLC
                        dlcState = DS_CLEANING;
                    }
                    break;
                case EVENT_CHECK_ERROR:
                    dlcError = DE_CHECK_ERROR;
                    dlcState = DS_DONE;
                    break;
                case EVENT_CANCEL:
                    dlcState = DS_CANCELLING;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;

        case DS_CHECKING_PATCH:
            switch(event)
            {
                case EVENT_CHECK_OK:
                    // automatically start download after check?
                    if(fsmAutoReady)
                    {
                        dlcState = DS_DOWNLOADING;
                    }
                    else
                    {
                        dlcState = DS_READY;
                    }
                    break;
                case EVENT_CHECK_ERROR:
                    dlcError = DE_CHECK_ERROR;
                    dlcState = DS_DONE;
                    break;
                case EVENT_CANCEL:
                    dlcState = DS_CANCELLING;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;

        case DS_READY:
            switch(event)
            {
                case EVENT_DOWNLOAD_START:
                    dlcState = DS_DOWNLOADING;
                    break;
                case EVENT_CANCEL:
                    dlcError = DE_WAS_CANCELED;
                    dlcState = DS_DONE;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;

        case DS_DOWNLOADING:
            switch(event)
            {
                case EVENT_DOWNLOAD_OK:
                    dlcState = DS_PATCHING;
                    break;
                case EVENT_DOWNLOAD_ERROR:
                    dlcError = DE_DOWNLOAD_ERROR;
                    dlcState = DS_DONE;
                    break;
                case EVENT_CANCEL:
                    dlcState = DS_CANCELLING;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;
        case DS_PATCHING:
            switch(event)
            {
                case EVENT_PATCH_OK:
                    // a new version can appear while we doing patching
                    // so we need to check it once again
                    fsmAutoReady = true;
                    dlcState = DS_CHECKING_INFO;
                    break;
                case EVENT_PATCH_ERROR_LITE:
                    dlcError = DE_PATCH_ERROR_LITE;
                    dlcState = DS_DONE;
                    break;
                case EVENT_PATCH_ERROR_FULL:
                    dlcError = DE_PATCH_ERROR_FULL;
                    dlcState = DS_DONE;
                    break;
                case EVENT_CANCEL:
                    dlcState = DS_CANCELLING;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;
        case DS_CANCELLING:
            switch(event)
            {
                case EVENT_CHECK_OK:
                case EVENT_CHECK_ERROR:
                case EVENT_DOWNLOAD_OK:
                case EVENT_DOWNLOAD_ERROR:
                case EVENT_PATCH_OK:
                case EVENT_PATCH_ERROR_LITE:
                case EVENT_PATCH_ERROR_FULL:
                case EVENT_CANCEL_OK:
                    dlcError = DE_WAS_CANCELED;
                    dlcState = DS_DONE;
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;
        case DS_CLEANING:
            switch(event)
            {
                case EVENT_CLEAN_OK:
                    dlcState = DS_DONE;
                    break;
                case EVENT_CANCEL:
                    break;
                default:
                    eventHandled = false;
                    break;
            }
            break;
        case DS_DONE:
            break;
        default:
            break;
    }

    if(!eventHandled)
    {
        Logger::Error("Unhanded event %d in state %d\n", event, dlcState);
        DVASSERT(false); 
    }

    // what should we do, when state changed
    if(oldState != dlcState)
    {
        switch(dlcState)
        {
            case DS_INIT:
                break;
            case DS_CHECKING_INFO:
                WriteUint32(dlcContext.stateInfoStorePath, dlcState);
                StepCheckInfoBegin();
                break;
            case DS_CHECKING_PATCH:
                StepCheckPatchBegin();
                break;
            case DS_READY:
                break;
            case DS_DOWNLOADING:
                WriteUint32(dlcContext.stateInfoStorePath, dlcState);
                StepDownloadPatchBegin();
                break;
            case DS_PATCHING:
                WriteUint32(dlcContext.stateInfoStorePath, dlcState);
                StepPatchBegin();
                break;
            case DS_CANCELLING:
                switch(oldState)
                {
                    case DS_CHECKING_INFO:
                        StepCheckInfoCancel();
                        break;
                    case DS_CHECKING_PATCH:
                        StepCheckPatchCancel();
                        break;
                    case DS_READY:
                        break;
                    case DS_DOWNLOADING:
                        StepDownloadPatchCancel();
                        break;
                    case DS_PATCHING:
                        StepPatchCancel();
                        break;
                    default:
                        Logger::Error("Unhanded state canceling\n");
                        DVASSERT(false);
                        break;
                }
                break;
            case DS_CLEANING:
                StepClean();
                break;
            case DS_DONE:
                StepDone();
                break;
            default:
                break;
        }
    }
}

// start downloading remove DLC version 
void DLC::StepCheckInfoBegin()
{
    if(!dlcContext.forceFullUpdate)
    {
        dlcContext.localVer = ReadUint32(dlcContext.localVerStorePath);
    }

    DataDownloadManager::Instance()->SetNotificationCallback(DataDownloadManager::NotifyFunctor(this, &DLC::StepCheckInfoFinish));
    dlcContext.remoteVerDownloadId = DataDownloadManager::Instance()->Download(dlcContext.remoteVerUrl, dlcContext.remoteVerStotePath.GetAbsolutePathname(), FULL);   
}

// downloading DLC version file finished. need to read removeVersion
void DLC::StepCheckInfoFinish(const uint32 &id, const DownloadStatus &status)
{
    if(id == dlcContext.remoteVerDownloadId)
    {
        if(DL_FINISHED == status)
        {
            DownloadError downloadError;
            DataDownloadManager::Instance()->GetError(id, downloadError);

            if(downloadError == DLE_NO_ERROR && dlcContext.remoteVerStotePath.Exists())
            {
                dlcContext.remoteVer = ReadUint32(dlcContext.remoteVerStotePath);
                FSM(EVENT_CHECK_OK);
            }
            else
            {
                FSM(EVENT_CHECK_ERROR);
            }
        }
    }
}

void DLC::StepCheckInfoCancel()
{
    DataDownloadManager::Instance()->Cancel(dlcContext.remoteVerDownloadId);
}

void DLC::StepCheckPatchBegin()
{
    dlcContext.remotePatchFullUrl = dlcContext.remoteUrl + MakePatchUrl(0, dlcContext.remoteVer);
    dlcContext.remotePatchLiteUrl = dlcContext.remoteUrl + MakePatchUrl(dlcContext.localVer, dlcContext.remoteVer);

    DataDownloadManager::Instance()->SetNotificationCallback(DataDownloadManager::NotifyFunctor(this, &DLC::StepCheckPatchFinish));
    dlcContext.remoteFullSizeDownloadId = DataDownloadManager::Instance()->GetSize(dlcContext.remotePatchFullUrl, 2000, 2); // full size should be first
    dlcContext.remoteLiteSizeDownloadId = DataDownloadManager::Instance()->GetSize(dlcContext.remotePatchLiteUrl, 2000, 2); // lite size should be last
}

void DLC::StepCheckPatchFinish(const uint32 &id, const DownloadStatus &status)
{
    if(id == dlcContext.remoteLiteSizeDownloadId)
    {
        if(DL_FINISHED == status)
        {
            DownloadStatus statusFull;
            DownloadError downloadErrorFull;
            DownloadError downloadErrorLite;

            DataDownloadManager::Instance()->GetStatus(dlcContext.remoteFullSizeDownloadId, statusFull);
            DataDownloadManager::Instance()->GetError(dlcContext.remoteFullSizeDownloadId, downloadErrorFull);
            DataDownloadManager::Instance()->GetError(dlcContext.remoteLiteSizeDownloadId, downloadErrorLite);

            // when lite id finishing, full id should be already finished
            DVASSERT(DL_FINISHED == statusFull);
    
            if(downloadErrorLite == DLE_NO_ERROR)
            {
                dlcContext.remotePatchUrl = dlcContext.remotePatchLiteUrl;
                DataDownloadManager::Instance()->GetTotal(dlcContext.remoteLiteSizeDownloadId, dlcContext.remotePatchSize);

                FSM(EVENT_CHECK_OK);
            }
            else
            {
                if(DL_FINISHED == statusFull && DLE_NO_ERROR == downloadErrorFull)
                {
                    dlcContext.remotePatchUrl = dlcContext.remotePatchFullUrl;
                    DataDownloadManager::Instance()->GetTotal(dlcContext.remoteFullSizeDownloadId, dlcContext.remotePatchSize);

                    FSM(EVENT_CHECK_OK);
                }
                else
                {
                    FSM(EVENT_CHECK_ERROR);
                }
            }
        }
    }
}

void DLC::StepCheckPatchCancel()
{
    DataDownloadManager::Instance()->Cancel(dlcContext.remoteFullSizeDownloadId);
    DataDownloadManager::Instance()->Cancel(dlcContext.remoteLiteSizeDownloadId);
}

// download patch file
void DLC::StepDownloadPatchBegin()
{
    // last state was 'Patching'?
    if(dlcContext.prevState == DS_PATCHING && dlcContext.remotePatchStorePath.Exists())
    {
        // we should without downloading
        FSM(EVENT_DOWNLOAD_OK);
    }
    else
    {
        // what mode should be used for download?
        // by default - full download
        DownloadType donwloadType = FULL;

        // check what URL was downloaded last time
        File *downloadInfoFile = File::Create(dlcContext.downloadInfoStorePath, File::OPEN | File::READ);
        if(NULL != downloadInfoFile)
        {
            String lastUrl;
            String lastSizeStr;
            uint32 lastSize;

            downloadInfoFile->ReadString(lastSizeStr);
            downloadInfoFile->ReadString(lastUrl);

            lastSize = atoi(lastSizeStr.c_str());

            // last url is same as we are trying to download now
            if(lastUrl == dlcContext.remotePatchUrl && lastSize == dlcContext.remotePatchSize)
            {
                // now we can resume last download
                donwloadType = AUTO;
            }

            SafeRelease(downloadInfoFile);
        }

        // save URL that we gonna download
        downloadInfoFile = File::Create(dlcContext.downloadInfoStorePath, File::CREATE | File::WRITE);
        if(NULL != downloadInfoFile)
        {
            String sizeStr = Format("%u", dlcContext.remotePatchSize);
            downloadInfoFile->WriteString(sizeStr);
            downloadInfoFile->WriteString(dlcContext.remotePatchUrl);
            SafeRelease(downloadInfoFile);
        }

        // start download and notify about download status into StepDownloadPatchFinish
        DataDownloadManager::Instance()->SetNotificationCallback(DataDownloadManager::NotifyFunctor(this, &DLC::StepDownloadPatchFinish));
        dlcContext.remotePatchDownloadId = DataDownloadManager::Instance()->Download(dlcContext.remotePatchUrl, dlcContext.remotePatchStorePath.GetAbsolutePathname(), donwloadType);
    }
}

void DLC::StepDownloadPatchFinish(const uint32 &id, const DownloadStatus &status)
{
    if(id == dlcContext.remotePatchDownloadId)
    {
        if(DL_FINISHED == status)
        {
            DownloadError downloadError;
            DataDownloadManager::Instance()->GetError(id, downloadError);

            if(downloadError == DLE_NO_ERROR)
            {
                FSM(EVENT_DOWNLOAD_OK);
            }
            else
            {
                FSM(EVENT_DOWNLOAD_ERROR);
            }
        }
    }
}

void DLC::StepDownloadPatchCancel()
{
    DataDownloadManager::Instance()->Cancel(dlcContext.remotePatchDownloadId);
}

void DLC::StepPatchBegin()
{
    dlcContext.patchingOk = true;
    dlcContext.patchIndex = 0;
    dlcContext.patchCount = 0;

    // read number of available patches
    PatchFileReader patchReader(dlcContext.remotePatchStorePath);
    if(patchReader.ReadFirst())
    {
        do
        {
            dlcContext.patchCount++;
        }
        while(patchReader.ReadNext());
    }

    patchingThread = Thread::Create(Message(this, &DLC::PatchingThread));
    patchingThread->Start();
}

void DLC::StepPatchFinish(BaseObject *caller, void *callerData, void *userData)
{
    patchingThread->Join();
    SafeRelease(patchingThread);

    dlcContext.prevState = 0;
    dlcContext.localVer = -1;

    if(dlcContext.patchingOk)
    {
        FSM(EVENT_PATCH_OK);
    }
    else
    {
        if(dlcContext.remotePatchUrl == dlcContext.remotePatchFullUrl)
        {
            FSM(EVENT_PATCH_ERROR_FULL);
        }
        else
        {
            FSM(EVENT_PATCH_ERROR_LITE);
        }
    }
}

void DLC::StepPatchCancel()
{
    dlcContext.patchingOk = false;
}

void DLC::PatchingThread(BaseObject *caller, void *callerData, void *userData)
{
    PatchFileReader patchReader(dlcContext.remotePatchStorePath);
    if(patchReader.ReadFirst())
    {
        do
        {
            PatchFileReader::PatchError patchError = patchReader.Apply(dlcContext.localSourceDir, FilePath(), dlcContext.localDestinationDir, FilePath());
            if(PatchFileReader::ERROR_NO != patchError && PatchFileReader::ERROR_PATCHED != patchError)
            {
                dlcContext.patchingOk = false;
            }

            dlcContext.patchIndex++;
        }
        while(dlcContext.patchingOk && patchReader.ReadNext());

        if(dlcContext.patchingOk)
        {
            // update local version
            WriteUint32(dlcContext.localVerStorePath, dlcContext.remoteVer);
        }
    }
    else
    {
        dlcContext.patchingOk = false;
    }

    JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &DLC::StepPatchFinish));
}

void DLC::StepClean()
{
    FileSystem::Instance()->DeleteFile(dlcContext.downloadInfoStorePath);
    FileSystem::Instance()->DeleteFile(dlcContext.remotePatchStorePath);

    FSM(EVENT_CLEAN_OK);
}

void DLC::StepDone()
{
    FileSystem::Instance()->DeleteFile(dlcContext.remoteVerStotePath);

    if(DE_NO_ERROR == dlcError)
    {
        FileSystem::Instance()->DeleteFile(dlcContext.stateInfoStorePath);
    }
}

uint32 DLC::ReadUint32(const FilePath &path)
{
    uint32 ret = 0;

    // check if there is some unfinished download
    File *f = File::Create(path, File::OPEN | File::READ);
    if(NULL != f)
    {
        char8 tmp[64];
        tmp[0] = 0;
        f->ReadLine(tmp, sizeof(tmp));
        ret = atoi(tmp);
        SafeRelease(f);
    }

    return ret;
}

void DLC::WriteUint32(const FilePath &path, uint32 value)
{
    // check if there is some unfinished download
    File *f = File::Create(path, File::CREATE | File::WRITE);
    if(NULL != f)
    {
        String tmp = Format("%u", value);
        f->WriteLine(tmp);
        SafeRelease(f);
    }
}

String DLC::MakePatchUrl(uint32 localVer, uint32 remoteVer)
{
    String ret;

    eGPUFamily gpu = DeviceInfo::GetGPUFamily();
    if(gpu < GPU_FAMILY_COUNT)
    {
        String gpuString = GPUFamilyDescriptor::GetGPUName(gpu);
        ret = Format("/r%u/r%u-%u.%s.patch", remoteVer, localVer, remoteVer, gpuString.c_str());
    }
    else
    {
        ret = Format("/r%u/r%u-%u.patch", remoteVer, localVer, remoteVer);
    }

    return ret;
}

}
