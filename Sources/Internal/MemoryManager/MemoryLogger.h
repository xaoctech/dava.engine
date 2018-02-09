#pragma once

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include "Base/BaseTypes.h"

namespace DAVA
{
class File;
class MemoryManager;

class MemoryLogger
{
public:
    static const uint32 FILE_SIGNATURE = 0x41764144;
    struct FileHeader
    {
        uint32 signature;
        uint32 statCount;
        uint32 finished;
        uint32 devInfoSize;
        uint32 statConfigSize;
        uint32 statItemSize;
        uint32 padding[2];
    };
    static_assert(sizeof(FileHeader) == 32, "sizeof(FileHeader) != 32");

    MemoryLogger();
    ~MemoryLogger();

    void OnMemoryManagerUpdate();
    void OnMemoryManagerTag(uint32 tag, bool tagEnterFlag);
    void OnMemoryManagerFinish();

private:
    bool CreateLogFile();
    void FlushLogFile();
    void SaveSnapshotOnQuit();
    void ReportStatOnQuit();

    static const uint32 maxItemsInBuffer = 600;

    MemoryManager* mm = nullptr;
    uint32 statItemSize = 0;
    uint32 bufferSize = 0;
    std::unique_ptr<uint8[]> buffer;
    uint32 nitemsInBuffer = 0;
    uint32 nitemsWritten = 0;

    File* logFile = nullptr;
    File* snapshotFile = nullptr;
    uint32 configSize = 0;
    uint32 peerDescrSize = 0;
    int64 baseTimestamp = 0;
};

} // namespace DAVA

#endif // defined(DAVA_MEMORY_PROFILING_ENABLE)
