#pragma once

#include "Concurrency/Mutex.h"
#include "FileWatcherImpl.h"
#if FILEWATCHER_PLATFORM == FILEWATCHER_PLATFORM_KQUEUE
#include <CoreServices/CoreServices.h>

namespace FW
{
class FileWatcherOSX : public FileWatcherImpl
{
public:
    FileWatcherOSX();

    virtual ~FileWatcherOSX();

    WatchID addWatch(const String& directory, FileWatchListener* listener, bool recursive);

    void removeWatch(const String& directory);

    void removeWatch(WatchID watchid);

    void update();

    void handleAction(WatchStruct* watch, const String& filename, unsigned long action){};

private:
    struct Watcher
    {
        WatchID id = 0;
        FileWatchListener* listener = nullptr;
        std::string rootDir;
        std::vector<std::string> alldirs;

        void CreateFSEvents();
        void ReleaseFSEvents();

        FSEventStreamContext* context = nullptr;
        FSEventStreamRef stream = nullptr;
        static void FSEventsCallback(ConstFSEventStreamRef streamRef,
                                     void* clientCallBackInfo,
                                     size_t numEvents,
                                     void* eventPaths,
                                     const FSEventStreamEventFlags eventFlags[],
                                     const FSEventStreamEventId eventIds[]);
    };

    std::map<WatchID, Watcher*> watchers;
    WatchID watchIDCounter = 1;
    bool restart = true;
};
};

#endif //__APPLE_CC__
