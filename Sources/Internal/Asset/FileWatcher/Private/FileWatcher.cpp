/**
	Copyright (c) 2009 James Wynn (james@jameswynn.com)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#include "Asset/FileWatcher/FileWatcher.h"
#include "Asset/FileWatcher/FileWatcherImpl.h"
#include "Engine/Engine.h"
#include "Job/JobManager.h"

#if FILEWATCHER_PLATFORM == FILEWATCHER_PLATFORM_WIN32
#include "Asset/FileWatcher/FileWatcherWin32.h"
#define FILEWATCHER_IMPL FileWatcherWin32
#elif FILEWATCHER_PLATFORM == FILEWATCHER_PLATFORM_KQUEUE
#include "Asset/FileWatcher/FileWatcherOSX.h"
#define FILEWATCHER_IMPL FileWatcherOSX
#elif FILEWATCHER_PLATFORM == FILEWATCHER_PLATFORM_LINUX
#include "Asset/FileWatcher/FileWatcherLinux.h"
#define FILEWATCHER_IMPL FileWatcherLinux
#endif

namespace FW
{
FileWatchListenerToSignal::FileWatchListenerToSignal(DAVA::Signal<WatchID, String, String, Action>& originalSignal_)
    : originalSignal(originalSignal_)
{
}

FileWatchListenerToSignal::~FileWatchListenerToSignal()
{
}

void FileWatchListenerToSignal::handleFileAction(WatchID watchid, const String& dir, const String& filename, Action action)
{
    DAVA::GetEngineContext()->jobManager->CreateMainJob
    ([=] {
        originalSignal.Emit(watchid, dir, filename, action);
    });
}

//--------
FileWatcher::FileWatcher()
    : watcher(onWatchersChanged)
{
    mImpl = new FILEWATCHER_IMPL();
}

//--------
FileWatcher::~FileWatcher()
{
    FinishUpdateThread();
}

//--------
WatchID FileWatcher::AddWatch(const String& directory)
{
    return mImpl->addWatch(directory, &watcher, false);
}

//--------
WatchID FileWatcher::AddWatch(const String& directory, bool recursive)
{
    return mImpl->addWatch(directory, &watcher, recursive);
}

//--------
void FileWatcher::RemoveWatch(const String& directory)
{
    mImpl->removeWatch(directory);
}

//--------
void FileWatcher::RemoveWatch(WatchID watchid)
{
    mImpl->removeWatch(watchid);
}

//--------
void FileWatcher::Update()
{
    mImpl->update();
}

/// Create update thread
void FileWatcher::StartUpdateThread()
{
    finishThread = false;
    DVASSERT(updateThread == nullptr);
    DAVA::Function<void()> threadUpdateFunc = DAVA::MakeFunction(this, &FileWatcher::ThreadUpdateFunc);
    updateThread = DAVA::Thread::Create(threadUpdateFunc);
    updateThread->Start();
}

void FileWatcher::ThreadUpdateFunc()
{
    while (!finishThread)
    {
        Update();
        updateThread->Sleep(100);
    }

    delete mImpl;
    mImpl = 0;
}

/// Finish update thread
void FileWatcher::FinishUpdateThread()
{
    if (updateThread)
    {
        finishThread = true;
        if (updateThread->IsJoinable())
        {
            updateThread->Join();
        }
        updateThread->Release();
        updateThread = nullptr;
    }
}

}; //namespace FW
