#include "NetworkCore/SnapshotStat.h"

#include <Base/StringStream.h>
#include <Debug/Backtrace.h>
#include <Engine/Engine.h>
#include <Entity/ComponentManager.h>
#include <Logger/Logger.h>
#include <Time/SystemTimer.h>

namespace DAVA
{
SnapshotStat* serverSnapshotStat = nullptr;
SnapshotStat* clientSnapshotStat = nullptr;

void SnapshotStat::Dump()
{
    StringStream ss;
    ComponentManager* componentManager = GetEngineContext()->componentManager;

    ss << "generalstat:" << size << ';' << ntotal << ';' << nempty << ';' << nfull << std::endl;
    for (const auto& pair : componentStat)
    {
        const uint32& componentId = pair.first;
        const ComponentStatItem& item = pair.second;

        if (item.count > 0)
        {
            String name = Debug::DemangleFrameSymbol(componentManager->GetSceneComponentType(componentId)->GetName());
            ss << "componentstat:" << componentId << ';' << name << ';' << item.count << ';' << item.countNew << ';' << item.bits << ';' << item.bitsPayload << std::endl;
            for (const auto& pair : item.typeStat)
            {
                const FastName& name = pair.first;
                const TypeStatItem& item = pair.second;

                if (item.count > 0)
                {
                    ss << "typestat:" << componentId << ';' << name.c_str() << ';' << item.count << ';' << item.countNew << ';' << item.bits << std::endl;
                }
            }
        }
    }
    Logger::Debug("%s", ss.str().c_str());
}

void SnapshotStat::PeriodicDump()
{
    if (lastStatDump == 0)
        lastStatDump = SystemTimer::GetMs();

    int64 curTimestamp = SystemTimer::GetMs();
    int64 timeDelta = curTimestamp - lastStatDump;
    if (timeDelta >= 1000 * 11)
    {
        Dump();
        lastStatDump = curTimestamp;

        size = 0;
        ntotal = 0;
        nempty = 0;
        nfull = 0;
        for (auto& pair : componentStat)
        {
            ComponentStatItem& item = pair.second;
            item.count = 0;
            item.countNew = 0;
            item.bits = 0;
            item.bitsPayload = 0;
            for (auto& pair : item.typeStat)
            {
                TypeStatItem& item = pair.second;
                item.count = 0;
                item.countNew = 0;
                item.bits = 0;
            }
        }
    }
}

} // namespace DAVA
