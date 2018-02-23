#include "Asset/FileWatcher/FileWatcherOSX.h"
#include "Concurrency/LockGuard.h"

#if FILEWATCHER_PLATFORM == FILEWATCHER_PLATFORM_KQUEUE

#include <sys/event.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

namespace FW
{
DAVA::Mutex mutex;

FileWatcherOSX::FileWatcherOSX()
{
}

FileWatcherOSX::~FileWatcherOSX()
{
    DAVA::LockGuard<DAVA::Mutex> guard(mutex);
    for (auto& it : watchers)
        it.second->ReleaseFSEvents();
}

void FileWatcherOSX::Watcher::ReleaseFSEvents()
{
    if (stream)
    {
        FSEventStreamStop(stream);
        FSEventStreamInvalidate(stream);
        FSEventStreamRelease(stream);

        stream = nullptr;
    }
}

void FileWatcherOSX::Watcher::CreateFSEvents()
{
    if (stream != nullptr)
        return;

    std::vector<CFStringRef> dirs;
    for (const std::string& path : alldirs)
    {
        dirs.push_back(CFStringCreateWithCString(nullptr,
                                                 path.c_str(),
                                                 kCFStringEncodingUTF8));
    }

    if (dirs.empty())
        return;

    CFArrayRef pathsToWatch =
    CFArrayCreate(nullptr,
                  reinterpret_cast<const void**>(&dirs[0]),
                  dirs.size(),
                  &kCFTypeArrayCallBacks);

    context = new FSEventStreamContext();
    context->version = 0;
    context->info = this;
    context->retain = nullptr;
    context->release = nullptr;
    context->copyDescription = nullptr;

    stream = FSEventStreamCreate(nullptr,
                                 &FileWatcherOSX::Watcher::FSEventsCallback,
                                 context,
                                 pathsToWatch,
                                 kFSEventStreamEventIdSinceNow,
                                 1.0f,
                                 kFSEventStreamCreateFlagFileEvents);

    if (!stream)
        throw Exception("Event stream could not be created.");

    // Fire the event loop
    CFRunLoopRef run_loop = CFRunLoopGetCurrent();
    if (!run_loop)
        throw Exception("Run loop could not be retreived");

    // Loop Initialization

    FSEventStreamScheduleWithRunLoop(stream,
                                     run_loop,
                                     kCFRunLoopDefaultMode);

    FSEventStreamStart(stream);
}

WatchID FileWatcherOSX::addWatch(const String& startDirectory, FileWatchListener* listener, bool recursive)
{
    DAVA::LockGuard<DAVA::Mutex> guard(mutex);

    Watcher* watcher = new Watcher();
    watcher->listener = listener;
    watcher->rootDir = startDirectory;
    watcher->id = watchIDCounter;

    if (recursive)
    {
        std::stack<std::string> directoryStack;
        directoryStack.push(startDirectory);
        while (!directoryStack.empty())
        {
            String currentDirectory = directoryStack.top();
            directoryStack.pop();

            // scan directory and call addFile(name, false) on each file
            DIR* dir = opendir(currentDirectory.c_str());
            if (!dir)
                throw FileNotFoundException(currentDirectory);

            if (currentDirectory != startDirectory)
            {
                watcher->alldirs.push_back(currentDirectory);
            }

            struct dirent* entry;
            struct stat attrib;
            while ((entry = readdir(dir)) != NULL)
            {
                String filename = String(entry->d_name);
                String fullFilepath = (currentDirectory + "/" + String(entry->d_name));
                stat(fullFilepath.c_str(), &attrib);
                if (S_ISDIR(attrib.st_mode))
                {
                    if ((filename != ".") && (filename != "..") && (filename != ".svn") && (filename != ".git"))
                        directoryStack.push(fullFilepath);
                }
            }

            closedir(dir);
        }
    }
    watchers.emplace(watchIDCounter, watcher);
    restart = true;

    return watchIDCounter++;
}

//--------
void FileWatcherOSX::removeWatch(const String& directory)
{
    for (auto it : watchers)
    {
        if (it.second->rootDir == directory)
        {
            removeWatch(it.first);
            break;
        }
    }
}

void FileWatcherOSX::removeWatch(WatchID watchid)
{
    DAVA::LockGuard<DAVA::Mutex> guard(mutex);
    auto it = watchers.find(watchid);
    if (it != watchers.end())
    {
        Watcher* watcher = it->second;
        watcher->ReleaseFSEvents();
        delete watcher;

        watchers.erase(it);
    }
}

void FileWatcherOSX::update()
{
    if (restart)
    {
        DAVA::LockGuard<DAVA::Mutex> guard(mutex);
        for (const auto& it : watchers)
            it.second->CreateFSEvents();
        restart = false;
    }

    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1f, 1);
}

void FileWatcherOSX::Watcher::FSEventsCallback(ConstFSEventStreamRef streamRef,
                                               void* clientCallBackInfo,
                                               size_t numEvents,
                                               void* eventPaths,
                                               const FSEventStreamEventFlags eventFlags[],
                                               const FSEventStreamEventId eventIds[])
{
    DAVA::LockGuard<DAVA::Mutex> guard(mutex);

    FileWatcherOSX::Watcher* watcher = static_cast<FileWatcherOSX::Watcher*>(clientCallBackInfo);

    if (!watcher)
    {
        throw Exception("The callback info cannot be cast to fsevents_monitor.");
    }

    time_t curr_time;
    time(&curr_time);

    for (size_t i = 0; i < numEvents; ++i)
    {
        //events.emplace_back(((char **) eventPaths)[i], curr_time, decode_flags(eventFlags[i]));
        char* path = ((char**)eventPaths)[i];
        if (eventFlags[i] & kFSEventStreamEventFlagItemCreated)
        {
            watcher->listener->handleFileAction(watcher->id, path, path, Action::Add);
        }
        else if (eventFlags[i] & kFSEventStreamEventFlagItemRemoved)
        {
            watcher->listener->handleFileAction(watcher->id, path, path, Action::Delete);
        }
        else if (eventFlags[i] & kFSEventStreamEventFlagItemModified)
        {
            watcher->listener->handleFileAction(watcher->id, path, path, Action::Modified);
        }

        /*kFSEventStreamEventFlagItemRenamed,
        kFSEventStreamEventFlagItemChangeOwner
        kFSEventStreamEventFlagItemXattrMod*/
    }
}
};

#endif //FILEWATCHER_PLATFORM_FSEVENTS
