#pragma once

#include <Base/Any.h>
#include <Base/Vector.h>
#include <Base/UnordererMap.h>
#include <Reflection/ReflectedMeta.h>
#include "NetworkTypes.h"

#define USE_SNAPSHOT_SYSTEM

// Logging level:
// 0 : no logs
// 1 : info logs
// 2 : verbose logs
#define LOG_SNAPSHOT_LEVEL 0

#if LOG_SNAPSHOT_LEVEL > 0
#define LOG_SNAPSHOT_SYSTEM(x) x
#else
#define LOG_SNAPSHOT_SYSTEM(x) 
#endif

#if LOG_SNAPSHOT_LEVEL > 1
#define LOG_SNAPSHOT_SYSTEM_VERBOSE(x) x
#else
#define LOG_SNAPSHOT_SYSTEM_VERBOSE(x) 
#endif

namespace DAVA
{
struct SnapshotComponentKey
{
    using UintT = uint16;

    UintT id = 0;
    UintT index = 0;

    SnapshotComponentKey() = default;
    SnapshotComponentKey(UintT id, UintT index);
    SnapshotComponentKey(uint32 id, uint32 index);
    SnapshotComponentKey& operator=(const SnapshotComponentKey&);
    bool operator<(const SnapshotComponentKey& key) const;
    bool operator==(const SnapshotComponentKey& key) const;
    bool operator!=(const SnapshotComponentKey& key) const;

    friend std::ostream& operator<<(std::ostream& stream, const SnapshotComponentKey& key);
};
} // namespace DAVA

namespace std
{
template <>
struct hash<DAVA::SnapshotComponentKey>
{
    inline size_t operator()(const DAVA::SnapshotComponentKey& x) const
    {
        static_assert(sizeof(x.id) + sizeof(x.index) <= sizeof(size_t), "Bad hash calculation");
        return (static_cast<size_t>(x.id) << std::numeric_limits<decltype(x.id)>::digits | static_cast<size_t>(x.index));
    }
};
} // namespace std

namespace DAVA
{
class Entity;
class Component;

struct SnapshotField final
{
    Any value;

    M::Privacy privacy;
    uint32 compression;
    float32 precision;
    float32 deltaPrecision; // Precision to make decision whether value has changed and should be sent over network
};

struct SnapshotComponent final
{
    M::Privacy privacy;
    Component* component = nullptr;
    Vector<SnapshotField> fields;

    void Fill(Component* component);
};

struct SnapshotEntity final
{
    using Children = Vector<NetworkID>;
    using Components = UnorderedMap<SnapshotComponentKey, SnapshotComponent>;

    Entity* entity = nullptr;
    Components components;
    Children children;
};

struct Snapshot final
{
    Snapshot();

    uint32 frameId = 0;
    UnorderedMap<NetworkID, SnapshotEntity> entities;
    UnorderedSet<NetworkID> rootEntities;

    const SnapshotEntity* FindEntity(NetworkID entityId) const;
    SnapshotEntity* FindEntity(NetworkID entityId);
    SnapshotEntity* AddEntity(Entity* entity, NetworkID parentId);
    void RemoveEntity(NetworkID parentEntityId, NetworkID entityId);
    void RemoveEntity(Entity* entity);
    void Init(uint32 frameId);
    void Clear();
};
} // namespace DAVA
