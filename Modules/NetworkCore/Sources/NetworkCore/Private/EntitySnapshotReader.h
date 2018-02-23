#pragma once

#include "NetworkCore/SnapshotUtils.h"

#include <Base/BaseTypes.h>

namespace DAVA
{
class BitReader;
struct Snapshot;

class EntitySnapshotReader
{
public:
    EntitySnapshotReader(BitReader& bitStream);
    EntitySnapshotReader(BitReader& bitStream, NetworkID rootEntityId, const Snapshot* snapshot1, Snapshot* snapshot2, SnapshotApplyCallback callback);
    size_t Read();
    size_t GetSize();

private:
    void ReadChildren(SnapshotEntity* rootEntity);
    void ReadComponents(NetworkID entityId, SnapshotEntity* entity);
    void ReadFieldsDelta(SnapshotComponentKey componentKey, Vector<SnapshotField>& fields);
    void ReadFields(SnapshotComponentKey componentKey, Vector<SnapshotField>& fields);

    SnapshotEntity* CopyEntitySnapshotRecursive(NetworkID entityId, NetworkID parentId);
    SnapshotEntity* CreateEntitySnapshot(Entity* entity, NetworkID parentId);

    BitReader& bstream;
    NetworkID rootEntityId;
    const Snapshot* snapshot1 = nullptr;
    Snapshot* snapshot2 = nullptr;
    SnapshotApplyCallback callback;
};

} // namespace DAVA
