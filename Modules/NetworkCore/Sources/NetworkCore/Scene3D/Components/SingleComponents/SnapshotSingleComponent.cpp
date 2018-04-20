#include "NetworkCore/Snapshot.h"
#include "NetworkCore/SnapshotUtils.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkReplicationSystem.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SnapshotSingleComponent)
{
    ReflectionRegistrator<SnapshotSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

namespace SnapshotSingleComponentDetails
{
Snapshot* GetSnapshot(SnapshotSingleComponent::SnapshotHistory& history, size_t& pos, uint32 frameId, bool create)
{
    Snapshot* ret = nullptr;

    DVASSERT(pos < history.size());
    uint32 curFrameId = history[pos].frameId;
    uint32 historySize = static_cast<uint32>(history.size());

    // requested current frame
    if (frameId == curFrameId)
    {
        ret = &history[pos];
    }
    // requested past frame
    else if (frameId < curFrameId)
    {
        uint32 offset = curFrameId - frameId;
        if (offset < historySize)
        {
            uint32 p = 0;
            if (offset <= pos)
            {
                p = static_cast<uint32>(pos - offset);
            }
            else
            {
                p = historySize - static_cast<uint32>(offset - pos);
            }

            // when creation requested, check if pos is free
            // and initialize it for specified frameId
            if (create && history[p].frameId == 0)
            {
                history[p].Init(frameId);
            }

            if (history[p].frameId == frameId)
            {
                ret = &history[p];
            }
        }
        else
        {
            // Can't create past frame, that is out of history bounds
            ret = nullptr;
        }
    }
    // requested future frame
    else
    {
        // can be get only if `create` flag is true
        if (create)
        {
            uint32 offset = frameId - curFrameId;
            if (offset < historySize)
            {
                for (size_t i = 0; i < offset; ++i)
                {
                    pos++; // override incoming `pos`
                    if (pos >= historySize)
                    {
                        pos = 0;
                    }

                    history[pos].Clear();
                    DVASSERT(history[pos].frameId == 0);
                }

                history[pos].Init(frameId);
                ret = &history[pos];
            }
            else
            {
                for (size_t i = 0; i < historySize; ++i)
                {
                    history[i].Clear();
                    DVASSERT(history[i].frameId == 0);
                }

                pos = historySize - 1; // override incoming `pos`

                history[pos].Init(frameId);
                ret = &history[pos];
            }
        }
    }

    return ret;
}
}

SnapshotSingleComponent::SnapshotSingleComponent()
{
    serverHistory.resize(128);
    clientHistory.resize(64);

    ResetServerHistory();
    ResetClientHistory();
}

Snapshot* SnapshotSingleComponent::GetServerSnapshot(uint32 frameId)
{
    return SnapshotSingleComponentDetails::GetSnapshot(serverHistory, serverHistoryPos, frameId, false);
}

Snapshot* SnapshotSingleComponent::GetClientSnapshot(uint32 frameId)
{
    return SnapshotSingleComponentDetails::GetSnapshot(clientHistory, clientHistoryPos, frameId, false);
}

Snapshot* SnapshotSingleComponent::CreateServerSnapshot(uint32 frameId)
{
    return SnapshotSingleComponentDetails::GetSnapshot(serverHistory, serverHistoryPos, frameId, true);
}

Snapshot* SnapshotSingleComponent::CreateClientSnapshot(uint32 frameId)
{
    return SnapshotSingleComponentDetails::GetSnapshot(clientHistory, clientHistoryPos, frameId, true);
}

void SnapshotSingleComponent::ResetServerHistory()
{
    size_t sz = serverHistory.size();
    for (size_t i = 0; i < sz; ++i)
    {
        serverHistory[i].Clear();
    }
    serverHistoryPos = 0;
}

void SnapshotSingleComponent::ResetClientHistory()
{
    size_t sz = clientHistory.size();
    for (size_t i = 0; i < sz; ++i)
    {
        clientHistory[i].Clear();
    }
    clientHistoryPos = 0;
}

bool SnapshotSingleComponent::ApplyServerDiff(ApplyDiffParams& params, SnapshotApplyCallback cb)
{
    static Snapshot dummySnapshot;
    static uint32 lastFrameId;

    DVASSERT(params.buff != nullptr);
    DVASSERT(params.buffSize > 0);
    DVASSERT(params.frameIdBase <= params.frameId);

    Snapshot* snapshot = CreateServerSnapshot(params.frameId);
    Snapshot* snapshotBase = GetServerSnapshot(params.frameIdBase);

    // we should reject diff if target snapshot didn't created
    if (nullptr == snapshot)
    {
        return false;
    }

    // we should reject incremental diff (non-zero base frameId)
    // in some cases... so check them
    if (params.frameIdBase != 0)
    {
        // reject if base snapshot isn't found
        if (nullptr == snapshotBase)
        {
            return false;
        }

        // reject if there is no appropriate entity in base snapshot
        if (nullptr == snapshotBase->FindEntity(params.entityId))
        {
            return false;
        }
    }

    params.outDiffSize = SnapshotUtils::ApplySnapshotDiff(snapshotBase, snapshot, params.entityId, params.buff, params.buffSize, cb);
    return (params.outDiffSize > 0);
}

} // namespace DAVA
