#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Base/UnordererMap.h>

#define COLLECT_CLIENT_SNAPSHOT_STAT

namespace DAVA
{
struct TypeStatItem
{
    uint32 count = 0;
    uint32 countNew = 0;
    uint32 bits = 0;
};

struct ComponentStatItem
{
    uint32 count = 0;
    uint32 countNew = 0;
    uint32 bits = 0;
    uint32 bitsPayload = 0;
    UnorderedMap<FastName, TypeStatItem> typeStat;
};

struct SnapshotStat
{
    uint32 size = 0; // size of all snapshots in bytes
    uint32 ntotal = 0; // total number of processed snapshots
    uint32 nempty = 0; // number of empty snapshots
    uint32 nfull = 0; // number of full sync snapshots
    UnorderedMap<uint32, ComponentStatItem> componentStat;

    void Dump();
    void PeriodicDump();

    int64 lastStatDump = 0;
};

extern SnapshotStat* serverSnapshotStat;
extern SnapshotStat* clientSnapshotStat;

} // namespace DAVA
