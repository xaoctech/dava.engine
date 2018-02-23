#pragma once

#include <Base/BaseTypes.h>
#include <Reflection/ReflectedMeta.h>

#include <utility>

namespace DAVA
{
class BitWriter;
struct Snapshot;

class EntitySnapshotWriter
{
public:
    EntitySnapshotWriter(BitWriter& bitStream, NetworkID rootEntityId, M::Privacy privacy, const Snapshot* snapshot1, const Snapshot* snapshot2);
    size_t Write();

private:
    bool WriteRootEntity(const SnapshotEntity* entity1, const SnapshotEntity* entity2);

    bool VisitChildren(NetworkID parentEntityId, const Vector<NetworkID>& children1, const Vector<NetworkID>& children2);
    bool WriteChildEntity(NetworkID parentEntityId, NetworkID entityId, const SnapshotEntity* entity1, const SnapshotEntity* entity2);

    bool WriteComponents(const SnapshotEntity::Components& components1, const SnapshotEntity::Components& components2);
    bool WriteComponentRemoved(SnapshotComponentKey componentKey);
    bool WriteComponentFieldsDiff(SnapshotComponentKey componentKey, const Vector<SnapshotField>& fields1, const Vector<SnapshotField>& fields2);
    bool WriteComponentFieldsFull(SnapshotComponentKey componentKey, const Vector<SnapshotField>& fields);

    bool CheckPrivacy(M::Privacy p) const;

    BitWriter& bstream;
    NetworkID rootEntityId;
    M::Privacy privacy = Metas::Privacy::PRIVATE;
    const Snapshot* snapshot1 = nullptr;
    const Snapshot* snapshot2 = nullptr;
};

inline bool EntitySnapshotWriter::CheckPrivacy(M::Privacy privacyToCheck) const
{
    return privacyToCheck >= privacy;
}

} // namespace DAVA
