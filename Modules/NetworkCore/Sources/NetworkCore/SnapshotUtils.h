#pragma once

#include "NetworkCore/Snapshot.h"

#include <Functional/Function.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedMeta.h>

#include <fstream>

namespace DAVA
{
class Component;
class Entity;

enum ObjectMarker : uint8
{
    MARKER_CHANGED,
    MARKER_ADDED,
    MARKER_REMOVED,
    MARKER_END,
};
//      MARKER_END | size present flag | full diff flag
constexpr uint8 EMPTY_DIFF = (ObjectMarker::MARKER_END << 2) | (0 << 1) | 0; // 0000 11           0                   0

enum class SnapshotApplyCommand
{
    ENTITY_ADDED,
    ENTITY_REMOVED,
    ENTITY_TOUCHED,

    COMPONENT_ADDED,
    COMPONENT_REMOVED,
    COMPONENT_CHANGED,
};

struct SnapshotApplyParam
{
    struct EntityParam
    {
        NetworkID parentId;
        NetworkID entityId;
        Entity* outEntity;
    };

    struct ComponentParam
    {
        NetworkID entityId;
        SnapshotComponentKey componentKey;
        Component* outComponent;
    };

    SnapshotApplyCommand cmd;

    union
    {
        EntityParam entityParam;
        ComponentParam componentParam;
    };
};

struct ReflectedComponentField
{
    Any key;
    const ValueWrapper* valueWrapper = nullptr;
    const M::Replicable* replicable = nullptr;
    uint32 compressionScheme = 0;
    float32 precision = 0.f;
    float32 deltaPrecision = 0.f;
};

using SnapshotApplyCallback = Function<void(SnapshotApplyParam& param)>;
using SnapshotApplyPredicate = Function<bool(SnapshotComponentKey, Component*)>;

struct SnapshotUtils
{
    static const Vector<ReflectedComponentField>& GetComponentFields(Reflection componentRef);

    static bool ApplySnapshot(Snapshot* snapshot, NetworkID entityId, Entity* dstEntity, SnapshotApplyPredicate pred = SnapshotApplyPredicate());
    static bool ApplySnapshot(Snapshot* snapshot, NetworkID entityId, SnapshotComponentKey componentKey, Component* dstComponent);
    static void ApplySnapshot(SnapshotComponent* snapshotComponent, Component* dstComponent);

    static bool TestSnapshotComponentsEqual(const SnapshotComponent* snapshotComponent1, const SnapshotComponent* snapshotComponent2);

    static size_t CreateSnapshotDiff(const Snapshot* base, Snapshot* current, NetworkID entityId, M::Privacy privacy, uint8* dstBuff, size_t dstSize);
    static size_t ApplySnapshotDiff(const Snapshot* base, Snapshot* target, NetworkID entityId, const uint8* srcBuff, size_t srcSize, SnapshotApplyCallback callback);
    static size_t GetSnapshotDiffSize(const uint8* srcBuff, size_t srcSize);

    static std::ostream& Log();
};

} // namespace DAVA
