#include "MemoryManager/MemoryLogger.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include "Base/TemplateHelpers.h"
#include "FileSystem/File.h"
#include "Logger/Logger.h"
#include "MemoryManager/MemoryManager.h"
#include "Network/PeerDesription.h"
#include "Time/SystemTimer.h"

namespace DAVA
{
MemoryLogger::MemoryLogger()
    : mm(MemoryManager::Instance())
    , statItemSize(mm->CalcCurStatSize())
    , bufferSize(statItemSize * maxItemsInBuffer)
    , buffer(std::make_unique<uint8[]>(bufferSize))
    , baseTimestamp(SystemTimer::GetMs())
{
    if (CreateLogFile())
    {
        // Create snapshot file in advance as in OnMemoryManagerFinish engine's subsystems do not exist
        snapshotFile = File::Create("onexit.snapshot", File::CREATE | File::WRITE);
        mm->SetCallbacks(MakeFunction(this, &MemoryLogger::OnMemoryManagerUpdate),
                         MakeFunction(this, &MemoryLogger::OnMemoryManagerTag),
                         MakeFunction(this, &MemoryLogger::OnMemoryManagerFinish));

        OnMemoryManagerUpdate();
    }
}

MemoryLogger::~MemoryLogger()
{
    OnMemoryManagerFinish();
}

void MemoryLogger::OnMemoryManagerUpdate()
{
    if (logFile != nullptr)
    {
        int64 curTime = SystemTimer::GetMs() - baseTimestamp;
        uint8* p = buffer.get() + nitemsInBuffer * statItemSize;
        mm->GetCurStat(curTime, p, statItemSize);
        nitemsInBuffer += 1;

        if (nitemsInBuffer == maxItemsInBuffer)
        {
            FlushLogFile();
            nitemsInBuffer = 0;
        }
    }
}

void MemoryLogger::OnMemoryManagerTag(uint32 tag, bool tagEnterFlag)
{
}

void MemoryLogger::OnMemoryManagerFinish()
{
    if (logFile != nullptr)
    {
        OnMemoryManagerUpdate();
        mm->SetCallbacks(nullptr, nullptr, nullptr);

        if (nitemsInBuffer > 0)
        {
            FlushLogFile();
            nitemsInBuffer = 0;
        }
        logFile->Release();
        logFile = nullptr;

        SaveSnapshotOnQuit();
        ReportStatOnQuit();
    }
}

bool MemoryLogger::CreateLogFile()
{
    logFile = File::Create("memory.mlog", File::CREATE | File::WRITE);
    if (logFile != nullptr)
    {
        FileHeader header{};
        Net::PeerDescription peerDescr;

        configSize = mm->CalcStatConfigSize();
        peerDescrSize = static_cast<uint32>(peerDescr.SerializedSize());

        std::unique_ptr<uint8[]> peerBuf = std::make_unique<uint8[]>(peerDescrSize);
        peerDescr.Serialize(peerBuf.get(), peerDescrSize);

        std::unique_ptr<uint8[]> configBuf = std::make_unique<uint8[]>(configSize);
        mm->GetStatConfig(configBuf.get(), configSize);

        logFile->Write(&header, sizeof(FileHeader));
        logFile->Write(peerBuf.get(), peerDescrSize);
        logFile->Write(configBuf.get(), configSize);
        logFile->Flush();
        return true;
    }
    return false;
}

void MemoryLogger::FlushLogFile()
{
    logFile->Write(buffer.get(), nitemsInBuffer * statItemSize);
    nitemsWritten += nitemsInBuffer;

    FileHeader header{};
    header.signature = FILE_SIGNATURE;
    header.statCount = nitemsWritten;
    header.finished = 1;
    header.devInfoSize = peerDescrSize;
    header.statConfigSize = configSize;
    header.statItemSize = statItemSize;

    logFile->Seek(0, File::SEEK_FROM_START);
    logFile->Write(&header, sizeof(FileHeader));
    logFile->Seek(0, File::SEEK_FROM_END);
    logFile->Flush();
}

void MemoryLogger::SaveSnapshotOnQuit()
{
    if (snapshotFile != nullptr)
    {
        int64 curTime = SystemTimer::GetMs() - baseTimestamp;
        mm->GetMemorySnapshot(curTime, snapshotFile, nullptr);

        snapshotFile->Release();
        snapshotFile = nullptr;
    }
}

void MemoryLogger::ReportStatOnQuit()
{
    mm->GetStatConfig(buffer.get(), configSize);
    mm->GetCurStat(0, buffer.get() + configSize, statItemSize);

    MMStatConfig* config = OffsetPointer<MMStatConfig>(buffer.get(), 0);
    MMItemName* poolNames = OffsetPointer<MMItemName>(config, sizeof(MMStatConfig));

    MMCurStat* stat = OffsetPointer<MMCurStat>(buffer.get(), configSize);
    AllocPoolStat* pools = OffsetPointer<AllocPoolStat>(stat, sizeof(MMCurStat));

    uint32 gpuTotalAlloc = pools[ALLOC_GPU_TEXTURE].allocByApp + pools[ALLOC_GPU_RDO_INDEX].allocByApp + pools[ALLOC_GPU_RDO_VERTEX].allocByApp;
    uint32 gpuTotalBlocks = pools[ALLOC_GPU_TEXTURE].blockCount + pools[ALLOC_GPU_RDO_INDEX].blockCount + pools[ALLOC_GPU_RDO_VERTEX].blockCount;

    pools[ALLOC_POOL_TOTAL].allocByApp -= gpuTotalAlloc;
    pools[ALLOC_POOL_TOTAL].blockCount -= gpuTotalBlocks;

    StringStream ss;
    for (uint32 i = 0; i < config->allocPoolCount; ++i)
    {
        if (i != ALLOC_GPU_TEXTURE && i != ALLOC_GPU_RDO_INDEX && i != ALLOC_GPU_RDO_VERTEX)
        {
            ss << "mprof:" << poolNames[i].name << ";" << pools[i].allocByApp << ";" << pools[i].blockCount << ";" << std::endl;
        }
    }
    ss << "mprof:gpu total;" << gpuTotalAlloc << ";" << gpuTotalBlocks << ";" << std::endl;
    for (auto i : { ALLOC_GPU_TEXTURE, ALLOC_GPU_RDO_INDEX, ALLOC_GPU_RDO_VERTEX })
    {
        ss << "mprof:" << poolNames[i].name << ";" << pools[i].allocByApp << ";" << pools[i].blockCount << ";" << std::endl;
    }
    String s = ss.str();

    Logger::PlatformLog(Logger::LEVEL_DEBUG, s.c_str());
}

} // namespace DAVA

#endif // defined(DAVA_MEMORY_PROFILING_ENABLE)
