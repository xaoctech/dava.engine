#include "NetworkCore/Snapshot.h"
#include "NetworkCore/SnapshotUtils.h"
#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Compression/Compression.h"

#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
Snapshot::Snapshot()
{
}

SnapshotEntity* Snapshot::FindEntity(NetworkID entityId)
{
    auto it = entities.find(entityId);
    if (it != entities.end())
    {
        return &it->second;
    }
    return nullptr;
}

const SnapshotEntity* Snapshot::FindEntity(NetworkID entityId) const
{
    auto it = entities.find(entityId);
    if (it != entities.end())
    {
        return &it->second;
    }
    return nullptr;
}

SnapshotEntity* Snapshot::AddEntity(Entity* entity, NetworkID parentId)
{
    SnapshotEntity* ret = nullptr;

    NetworkID entityId = NetworkCoreUtils::GetEntityId(entity);

    DVASSERT(entityId != NetworkID::INVALID);
    DVASSERT(parentId != NetworkID::INVALID);

    auto it = entities.find(entityId);
    if (it != entities.end())
    {
        ret = &it->second;

        DVASSERT(entities.find(parentId) != entities.end());
        DVASSERT(std::find(entities[parentId].children.begin(), entities[parentId].children.end(), entityId) != entities[parentId].children.end());
    }
    else
    {
        ret = &entities[entityId];
        ret->entity = entity;

        if (parentId == NetworkID::SCENE_ID)
        {
            rootEntities.insert(entityId);
        }
        else
        {
            DVASSERT(entities.find(parentId) != entities.end());

            entities[parentId].children.push_back(entityId);
            std::sort(entities[parentId].children.begin(), entities[parentId].children.end());
        }
    }

    return ret;
}

void Snapshot::RemoveEntity(NetworkID parentEntityId, NetworkID entityId)
{
    if (entityId != NetworkID::INVALID)
    {
        auto it = entities.find(entityId);
        if (it != entities.end())
        {
            entities.erase(it);
        }

        if (parentEntityId == NetworkID::SCENE_ID)
        {
            rootEntities.erase(entityId);
        }
        else
        {
            auto parentIt = entities.find(parentEntityId);
            if (parentIt != entities.end())
            {
                SnapshotEntity::Children* children = &parentIt->second.children;

                std::remove_if(children->begin(), children->end(), [entityId](NetworkID id) {
                    return (entityId == id);
                });
            }
        }
    }
}

void Snapshot::RemoveEntity(DAVA::Entity* entity)
{
    Entity* parent = entity->GetParent();

    NetworkID entityId = NetworkCoreUtils::GetEntityId(entity);
    NetworkID parentId = NetworkCoreUtils::GetEntityId(parent);
    RemoveEntity(parentId, entityId);
}

void Snapshot::Init(uint32 frameId_)
{
    DVASSERT(frameId == 0);
    DVASSERT(entities.size() == 0);
    DVASSERT(rootEntities.size() == 0);

    frameId = frameId_;
}

void Snapshot::Clear()
{
    frameId = 0;
    entities.clear();
    rootEntities.clear();
}

void SnapshotComponent::Fill(Component* component_)
{
    fields.clear();
    component = component_;

    // now add all component fields
    ReflectedObject compObj = ReflectedObject(component);
    Reflection compRef = Reflection::Create(compObj);

    DVASSERT(compRef.GetFieldsCaps().hasRangeAccess);
    DVASSERT(!compRef.GetFieldsCaps().hasDynamicStruct);

    // 1. get component reflection and meta
    const Vector<ReflectedComponentField>& fieldsRef = SnapshotUtils::GetComponentFields(compRef);

    const M::Replicable* compMeta = compRef.GetMeta<M::Replicable>();
    privacy = compMeta->privacy;

    // 2. add each field into snapshot
    // and also add them for change-watching
    fields.reserve(fieldsRef.size());
    for (size_t i = 0; i < fieldsRef.size(); ++i)
    {
        const M::Replicable* fieldReplicable = fieldsRef[i].replicable;
        M::Privacy fieldPrivacy = fieldReplicable->GetMergedPrivacy(privacy);

        // add field to snapshot snapshot
        {
            Any fieldValue = fieldsRef[i].valueWrapper->GetValue(compObj);

            SnapshotField sf;
            sf.value = std::move(fieldValue);
            sf.privacy = fieldPrivacy;
            sf.precision = fieldsRef[i].precision;
            sf.deltaPrecision = fieldsRef[i].deltaPrecision;
            sf.compression = fieldsRef[i].compressionScheme;
            fields.push_back(std::move(sf));
        }
    }
}

SnapshotComponentKey::SnapshotComponentKey(SnapshotComponentKey::UintT id_, SnapshotComponentKey::UintT index_)
    : id(id_)
    , index(index_)
{
}

SnapshotComponentKey::SnapshotComponentKey(uint32 id_, uint32 index_)
{
    DVASSERT(id_ < std::numeric_limits<SnapshotComponentKey::UintT>::max());
    DVASSERT(index_ < std::numeric_limits<SnapshotComponentKey::UintT>::max());

    id = static_cast<SnapshotComponentKey::UintT>(id_);
    index = static_cast<SnapshotComponentKey::UintT>(index_);
}

SnapshotComponentKey& SnapshotComponentKey::operator=(const SnapshotComponentKey& key)
{
    id = key.id;
    index = key.index;
    return *this;
}

bool SnapshotComponentKey::operator<(const SnapshotComponentKey& key) const
{
    return std::hash<SnapshotComponentKey>()(*this) < std::hash<SnapshotComponentKey>()(key);
}

bool SnapshotComponentKey::operator==(const SnapshotComponentKey& key) const
{
    return id == key.id && index == key.index;
}

bool SnapshotComponentKey::operator!=(const SnapshotComponentKey& key) const
{
    return !operator==(key);
}

} // namespace DAVA
