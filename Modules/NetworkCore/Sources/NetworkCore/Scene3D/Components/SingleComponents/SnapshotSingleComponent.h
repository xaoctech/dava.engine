#pragma once

#include "NetworkCore/Snapshot.h"
#include "NetworkCore/SnapshotUtils.h"
#include <Entity/SingletonComponent.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
namespace SnapshotSingleComponentDetails
{
class ServerDiffCache;
}

class SnapshotSingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(SnapshotSingleComponent, SingletonComponent);

    using SnapshotHistory = Vector<Snapshot>;

    SnapshotSingleComponent();

    size_t serverHistoryPos = 0;
    size_t clientHistoryPos = 0;
    SnapshotHistory serverHistory;
    SnapshotHistory clientHistory;

    Snapshot* GetServerSnapshot(uint32 frameId);
    Snapshot* GetClientSnapshot(uint32 frameId);

    Snapshot* CreateServerSnapshot(uint32 frameId);
    Snapshot* CreateClientSnapshot(uint32 frameId);

    void ResetServerHistory();
    void ResetClientHistory();

    struct CreateDiffParams
    {
        NetworkID entityId;
        uint32 frameIdBase = 0;
        uint32 frameId = 0;
        M::Privacy privacy = M::Privacy::PRIVATE;
        uint8* buff = nullptr;
        size_t buffSize = 0;

        size_t outDiffSize = 0;
        uint32 outFrameIdBase = 0;
    };

    struct ApplyDiffParams
    {
        NetworkID entityId;
        uint32 frameIdBase = 0;
        uint32 frameId = 0;
        const uint8* buff = nullptr;
        size_t buffSize = 0;

        size_t outDiffSize = 0;
    };

    bool GetServerDiff(CreateDiffParams& params);
    bool ApplyServerDiff(ApplyDiffParams& params, SnapshotApplyCallback cb);

private:
    std::unique_ptr<SnapshotSingleComponentDetails::ServerDiffCache> serverDiffCache;
};
} // namespace DAVA
