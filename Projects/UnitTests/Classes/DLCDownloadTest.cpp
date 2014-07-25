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

void DLCDownloadTest::TestFunction(PerfFuncData * data)
{
    String srcUrlMissing = "http://10.128.108.34:8080/missingFile.txt";
    String srcUrlMissingServer = "http://1.1.1.1/missingFile.txt";
    String srcUrl = "http://10.128.108.34:8080/file.msi";
    String srcUrlSSL = "https://10.128.108.34/file.msi";
    String srcUrlFTP = "ftp://10.128.108.34/ReSharperSetup.8.5.0.3585.msi";
    String srcUrlFTPUser = "ftp://test:test@10.128.108.34/ReSharperSetup.8.5.0.3585.msi";
    FilePath srcPathMissing(srcUrlMissing);
    FilePath srcServerMissing(srcUrlMissingServer);
    FilePath srcPath(srcUrl);
    FilePath srcPathSSL(srcUrlSSL);
    FilePath srcPathFTP(srcUrlFTP);
    FilePath srcPathFTPUser(srcUrlFTPUser);
    String dstMissing = "~doc:/downloads/res/"+srcPathMissing.GetFilename();
    String dstMissingServer = "~doc:/downloads/res/"+srcServerMissing.GetFilename();
    String dstHttp = "~doc:/downloads/res/"+srcPath.GetFilename();
    String dstHttps = "~doc:/downloads/res/ssl"+srcPathSSL.GetFilename();
    String dstFtp = "~doc:/downloads/res/ftp"+srcPathFTP.GetFilename();
    
    FileSystem::Instance()->DeleteFile(dstFtp);
    FileSystem::Instance()->DeleteFile(dstHttp);
    FileSystem::Instance()->DeleteFile(dstHttps);

    // set custom downloader - it interrupts download after one chunk of data comes
    DownloadManager::Instance()->SetDownloader(new CurlTestDownloader());
    
    // example "how to" usage of callback
    DownloadManager::Instance()->SetNotificationCallback(DownloadManager::NotifyFunctor(this, &DLCDownloadTest::DownloadCallback));

    DownloadStatus status;
    DownloadError error;
    uint64 progress = 0;
    uint64 total = 0;

    uint32 missingServerID  = DownloadManager::Instance()->Download(srcUrlMissingServer, dstMissingServer, RESUMED, 500, 1);
    DownloadManager::Instance()->Wait(missingServerID);

    uint32 missingID  = DownloadManager::Instance()->Download(srcUrlMissing, dstMissing);
    DownloadManager::Instance()->Wait(missingID);
    TEST_VERIFY(DownloadManager::Instance()->GetProgress(missingID, progress));
    TEST_VERIFY(DownloadManager::Instance()->GetTotal(missingID, total));
    DownloadManager::Instance()->GetError(missingID, error);
    TEST_VERIFY(DLE_CONTENT_NOT_FOUND == error);
    DownloadManager::Instance()->GetStatus(missingID, status);
    TEST_VERIFY(DL_FINISHED == status);

    // ask for some downloads
    uint32 ftpID  = DownloadManager::Instance()->Download(srcUrlFTP, "", GET_SIZE);
    DownloadManager::Instance()->Wait(ftpID);
    DownloadManager::Instance()->GetTotal(ftpID, total);
    TEST_VERIFY(total != 0);

    ftpID  = DownloadManager::Instance()->Download(srcUrlFTP, dstFtp);
    // wait for download
    DownloadManager::Instance()->Wait(ftpID);
    File *file = File::Create(dstFtp, File::OPEN | File::READ);
    TEST_VERIFY(NULL != file);
    uint64 filesize = file->GetSize();
    TEST_VERIFY(DownloadManager::Instance()->GetProgress(ftpID, progress));
    TEST_VERIFY(filesize == progress);
    SafeRelease(file);

    // change downloader to full featured Curl downloader.
    DownloadManager::Instance()->SetDownloader(new CurlDownloader());

    // wait for another download
    uint32 httpID  = DownloadManager::Instance()->Download(srcUrl, dstHttp);
    DownloadManager::Instance()->Wait(httpID);
    TEST_VERIFY(DownloadManager::Instance()->GetProgress(httpID, progress));
    TEST_VERIFY(DownloadManager::Instance()->GetTotal(httpID, total));
    file = File::Create(dstHttp, File::OPEN | File::READ);
    TEST_VERIFY(NULL != file);
    filesize = file->GetSize();
    TEST_VERIFY(filesize == progress);
    TEST_VERIFY(total == progress); 
    SafeRelease(file);

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

