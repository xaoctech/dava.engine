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

#include "DLCDownloadTest.h"
#include "DLC/Downloader/DownloadManager.h"
#include "DLC/Downloader/Downloader.h"
CurlTestDownloader::CurlTestDownloader(uint32 operationTimeout)
    : CurlDownloader(operationTimeout)
{
    
}

size_t CurlTestDownloader::SaveData(void *ptr, size_t size, size_t nmemb)
{
    static int8 i = 0;
    if (0 < i++)
    {
        Interrupt();
        i = 0;
        return 0;
    }
    
    return CurlDownloader::SaveData(ptr, size, nmemb);
}

DLCDownloadTest::DLCDownloadTest()
    : TestTemplate<DLCDownloadTest>("DLCDownloadTest")
    , serverUrl("by2-badava-mac-11.corp.wargaming.local")
    , testFileEmpty("/UnitTest/Downloader/empty.file")
    , testFileOne("/UnitTest/Downloader/r0-1.patch")
{
    RegisterFunction(this, &DLCDownloadTest::TestFunction, String("DLCDownloadTest TestFunction"), NULL);
}

void DLCDownloadTest::LoadResources()
{
    GetBackground()->SetColor(Color(1.f, 0, 0, 1));
}

void DLCDownloadTest::UnloadResources()
{
    RemoveAllControls();
}

void DLCDownloadTest::DownloadCallback(const uint32 &taskId, const DownloadStatus &status)
{
       Logger::FrameworkDebug("task %d status %d", taskId, status);
}

String DLCDownloadTest::StorePathForUrl(const String &url)
{
    String prefix = "";

    if (url.length() >= 5)
    {
        if ("ftp" == url.substr(0, 3))
        {
            prefix = "ftp";
        }

        else if ("ftps" == url.substr(0, 4))
        {
            prefix = "ftps";
        }
        else if ("http" == url.substr(0, 4))
        {
            prefix = "http";
        }
        else if ("https" == url.substr(0, 5))
        {
            prefix = "https";
        }
    }


    return "~doc:/downloads/res/" + prefix + "_" + FilePath(url).GetFilename();
}

void DLCDownloadTest::TestFunction(PerfFuncData * data)
{
    String srcUrlMissingFile = "http://" + serverUrl + "/missingFile.txt";
    String srcUrlFolder = "http://" + serverUrl + "/";
    String srcUrlEmptyFile = "http://" + serverUrl + testFileEmpty;
    String srcUrlMissingServer = "http://" + serverUrl + "/missingFile.txt";
    String srcUrl = "http://" + serverUrl + testFileOne;
    String srcUrlSSL = "https://" + serverUrl + testFileOne;
    String srcUrlFTP = "ftp://" + serverUrl + testFileOne;
    String srcUrlFTPUser = "ftp://" + serverUrl + testFileOne;

    String dstMissingFile = StorePathForUrl(srcUrlMissingFile);
    String dstHttpFolder = StorePathForUrl(srcUrlFolder);
    String dstHttpEmptyFile = StorePathForUrl(srcUrlEmptyFile);
    String dstMissingServer = StorePathForUrl(srcUrlMissingServer);
    String dstHttp = StorePathForUrl(srcUrl);
    String dstHttps = StorePathForUrl(srcUrlSSL);
    String dstFtp = StorePathForUrl(srcUrlFTP);

    FileSystem::Instance()->DeleteFile(dstHttp);

    // set custom downloader - it interrupts download after one chunk of data comes
    DownloadManager::Instance()->SetDownloader(new CurlTestDownloader());

    // example "how to" usage of callback
    DownloadManager::Instance()->SetNotificationCallback(DownloadManager::NotifyFunctor(this, &DLCDownloadTest::DownloadCallback));

    DownloadStatus status;
    DownloadError error;
    uint64 progress = 0;
    uint64 total = 0;
    uint64 filesize = 0;
    File *file = NULL;

    //uint32 missingServerID  = DownloadManager::Instance()->Download(srcUrlMissingServer, dstMissingServer, RESUMED, 500, 1);
    //DownloadManager::Instance()->Wait(missingServerID);
    /*
    uint32 missingID = DownloadManager::Instance()->Download(srcUrlMissingFile, dstMissingFile);
    DownloadManager::Instance()->Wait(missingID);
    TEST_VERIFY(DownloadManager::Instance()->GetProgress(missingID, progress));
    TEST_VERIFY(DownloadManager::Instance()->GetTotal(missingID, total));
    DownloadManager::Instance()->GetError(missingID, error);
    TEST_VERIFY(DLE_CONTENT_NOT_FOUND == error);
    DownloadManager::Instance()->GetStatus(missingID, status);
    TEST_VERIFY(DL_FINISHED == status);

    // ask for some downloads
    uint32 ftpID = DownloadManager::Instance()->Download(srcUrlFTP, "", GET_SIZE);
    DownloadManager::Instance()->Wait(ftpID);
    DownloadManager::Instance()->GetTotal(ftpID, total);
    TEST_VERIFY(total != 0);



    ftpID = DownloadManager::Instance()->Download(srcUrlFTP, dstFtp);
    // wait for download
    DownloadManager::Instance()->Wait(ftpID);
    file = File::Create(dstFtp, File::OPEN | File::READ);
    TEST_VERIFY(NULL != file);
    if (NULL != file)
    {
        filesize = file->GetSize();
        TEST_VERIFY(DownloadManager::Instance()->GetProgress(ftpID, progress));
        TEST_VERIFY(filesize == progress);
        SafeRelease(file);
    }


    */

/* POSITIVE CHECK */

    // change downloader to full featured Curl downloader.
    DownloadManager::Instance()->SetDownloader(new CurlDownloader());

    uint32 httpID = DownloadManager::Instance()->Download(srcUrl, dstHttp, RESUMED, 1000, 1);
    DownloadManager::Instance()->Wait(httpID);
    TEST_VERIFY(DownloadManager::Instance()->GetProgress(httpID, progress));
    TEST_VERIFY(DownloadManager::Instance()->GetTotal(httpID, total));
    file = File::Create(dstHttp, File::OPEN | File::READ);
    TEST_VERIFY(NULL != file);
    if (NULL != file)
    {
        filesize = file->GetSize();
        TEST_VERIFY(filesize == progress);
        TEST_VERIFY(total == progress);
        SafeRelease(file);
    }
    FileSystem::Instance()->DeleteFile(dstHttp);

/* END POSITIVE CHECK*/

/* MISSING FILE ON EXISTENT SERVER*/
    uint32 missingFileId = DownloadManager::Instance()->Download(srcUrlMissingFile, dstMissingFile, RESUMED, 1000, 0);
    DownloadManager::Instance()->Wait(missingFileId);

    DownloadManager::Instance()->GetError(missingFileId, error);
    TEST_VERIFY(DLE_CONTENT_NOT_FOUND == error);
    DownloadManager::Instance()->GetStatus(missingFileId, status);
    TEST_VERIFY(DL_FINISHED == status);

    // file should not be created
    file = File::Create(dstMissingFile, File::OPEN | File::READ);
    TEST_VERIFY(NULL == file);
    if (NULL != file)
    {
        FileSystem::Instance()->DeleteFile(dstMissingFile);
        SafeRelease(file);
    }

/* END MISSING FILE ON EXISTENT SERVER*/

/* FOLDER INSTEAD OF FILE */

    uint32 folderId = DownloadManager::Instance()->Download(srcUrlFolder, dstHttpFolder, RESUMED, 1000, 0);
    DownloadManager::Instance()->Wait(folderId);

    DownloadManager::Instance()->GetError(folderId, error);
    TEST_VERIFY(DLE_NO_ERROR == error);
    DownloadManager::Instance()->GetStatus(folderId, status);
    TEST_VERIFY(DL_FINISHED == status);

    FileSystem::Instance()->DeleteFile(dstHttpFolder);

/* END FOLDER INSTEAD OF FILE */

/* EMPTY FILE */

    uint32 emptyFileId = DownloadManager::Instance()->Download(srcUrlEmptyFile, dstHttpEmptyFile, RESUMED, 1000, 1);
    DownloadManager::Instance()->Wait(emptyFileId);
    TEST_VERIFY(DownloadManager::Instance()->GetProgress(emptyFileId, progress));
    TEST_VERIFY(DownloadManager::Instance()->GetTotal(emptyFileId, total));
    file = File::Create(dstHttpEmptyFile, File::OPEN | File::READ);
    TEST_VERIFY(NULL != file);
    if (NULL != file)
    {
        filesize = file->GetSize();
        TEST_VERIFY(filesize == progress);
        TEST_VERIFY(total == progress);
        SafeRelease(file);
    }
    FileSystem::Instance()->DeleteFile(dstHttpEmptyFile);

/* END EMPTY FILE */

    // ask for 3rd download
    uint32 httpsID = DownloadManager::Instance()->Download(srcUrlSSL, dstHttps);
    
    // interrupt task in process test. You should not call Update from your code, but that is unit test.
    DownloadManager::Instance()->Update();
    Thread::SleepThread(1000);
    DownloadManager::Instance()->Cancel(httpsID);
    DownloadManager::Instance()->GetError(httpsID, error);
    TEST_VERIFY(DLE_CANCELLED == error);
    DownloadManager::Instance()->GetStatus(httpsID, status);
    TEST_VERIFY(DL_FINISHED == status);

    DownloadManager::Instance()->Retry(httpsID);
    DownloadManager::Instance()->Update();
    Thread::SleepThread(1000);
    DownloadType type;
    DownloadManager::Instance()->GetType(httpsID, type);
    TEST_VERIFY(RESUMED == type);
    // cancell all downloads
    DownloadManager::Instance()->CancelAll();
    
    // download retry
    FileSystem::Instance()->DeleteFile(dstHttps);
    DownloadManager::Instance()->Retry(httpsID);

    // we don't need to full featured downloader here
    DownloadManager::Instance()->SetDownloader(new CurlTestDownloader);
    FileSystem::Instance()->DeleteFile(dstHttps);
    DownloadManager::Instance()->Retry(httpsID);
    DownloadManager::Instance()->Wait(httpsID);

    TEST_VERIFY(DownloadManager::Instance()->GetProgress(httpsID, progress));
    file = File::Create(dstHttps, File::OPEN | File::READ);
    TEST_VERIFY(NULL != file);
    filesize = file->GetSize();
    TEST_VERIFY(filesize == progress);
    SafeRelease(file);

    // download through https
    DownloadManager::Instance()->SetDownloader(new CurlDownloader);
    FileSystem::Instance()->DeleteFile(dstHttps);
    DownloadManager::Instance()->Retry(httpsID);
    DownloadManager::Instance()->Wait(httpsID);
    TEST_VERIFY(DownloadManager::Instance()->GetProgress(httpsID, progress));
    file = File::Create(dstHttps, File::OPEN | File::READ);
    TEST_VERIFY(NULL != file);
    filesize = file->GetSize();
    TEST_VERIFY(filesize == progress);

    SafeRelease(file);

    FileSystem::Instance()->DeleteFile(dstFtp);
    FileSystem::Instance()->DeleteFile(dstHttp);
    FileSystem::Instance()->DeleteFile(dstHttps);
}

