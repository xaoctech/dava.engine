#include "NetworkCore/Scene3D/Systems/SnapshotSystemBase.h"
#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/SnapshotUtils.h"
#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Compression/Compression.h"

#include <Math/Math2D.h>
#include <Scene3D/Scene.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedTypeDB.h>

#include <bitset>
#include <limits>
#include <algorithm>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SnapshotSystemBase)
{
    ReflectionRegistrator<SnapshotSystemBase>::Begin()
    .End();
}

SnapshotSystemBase::FieldWatchPoint::FieldWatchPoint(SnapshotField* snapshotField_, Entity* entity_, SnapshotComponentKey componentKey_, ReflectedObject refObject_, const ReflectedComponentField* refField_)
    : snapshotField(snapshotField_)
    , entity(entity_)
    , componentKey(componentKey_)
    , refObject(refObject_)
    , refField(refField_)
{
    entityId = NetworkCoreUtils::GetEntityId(entity);
    DVASSERT(entityId != NetworkID::INVALID);

    const Type* valueType = refField->valueWrapper->GetType(refObject)->Decay();
    DVASSERT(refObject.IsValid());
    DVASSERT(!valueType->IsPointer());

    cacheValue = refField->valueWrapper->GetValue(refObject);
    origSize = valueType->GetSize();
    origAddr = nullptr;
    isTrivial = valueType->IsTriviallyCopyable();

    if (isTrivial)
    {
        origAddr = refField->valueWrapper->GetValueObject(refObject).GetVoidPtr();
    }
}

bool SnapshotSystemBase::FieldWatchPoint::Update()
{
    bool changed = false;

    const void* cacheAddr = cacheValue.GetData();
    if (nullptr != origAddr)
    {
        if (::memcmp(cacheAddr, origAddr, origSize))
        {
            cacheValue.SetTrivially(origAddr, cacheValue.GetType());
            changed = true;
        }
    }
    else
    {
        Any v = refField->valueWrapper->GetValue(refObject);

        // we can use faster way for value stored in Any
        // if that value type is trivial
        if (!isTrivial)
        {
            // slower compare/assign operations
            // for non-trivial types
            if (cacheValue != v)
            {
                cacheValue = v;
                changed = true;
            }
        }
        else
        {
            // faster memcmp way
            // for trivial types
            const void* tmpOrigAddr = v.GetData();
            if (::memcmp(cacheAddr, tmpOrigAddr, origSize))
            {
                cacheValue.SetTrivially(tmpOrigAddr, cacheValue.GetType());
                changed = true;
            }
        }
    }

    return changed;
}

SnapshotSystemBase::SnapshotSystemBase(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkReplicationComponent>())
{
    // create snapshot for scene single components
    // it will have pre-defined entityId == NId2Scene
    snapshot.entities[NetworkID::SCENE_ID].entity = nullptr;

    for (auto& sc : scene->singletonComponents)
    {
        RegisterSingleComponent(sc.first);
    }

    // used for NetworkID generation, that should be right in snapshot
    //DVASSERT(nullptr != scene->GetSystem<NetworkIdSystem>());

    timeSingleComponent = scene->GetSingletonComponent<NetworkTimeSingleComponent>();
    snapshotSingleComponent = scene->GetSingletonComponent<SnapshotSingleComponent>();
}

SnapshotSystemBase::~SnapshotSystemBase()
{
}

void SnapshotSystemBase::UpdateSnapshot(NetworkID entityId)
{
    for (auto& wp : watchPoints)
    {
        if (entityId == NetworkID::INVALID || wp.entityId == entityId)
        {
            bool changed = wp.Update();
            if (changed)
            {
                wp.snapshotField->value = ApplyQuantization(wp.cacheValue, wp.snapshotField->compression, wp.snapshotField->deltaPrecision);

#if 0
                // send quantized value back to object
                if (wp.snapshotField->compression != 0)
                {
                    wp.refField->valueWrapper->SetValue(wp.refObject, wp.snapshotField->value);
                }
#endif

                // Log
                {
                    LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log()
                                                << timeSingleComponent->GetFrameId() << " | SnapChange "
                                                << wp.entityId << "."
                                                << wp.componentId << "."
                                                << wp.refField->key << " = "
                                                << wp.cacheValue << "\n");
                }
            }
        }
    }
}

void SnapshotSystemBase::RegisterEntity(Entity* entity)
{
    SceneSystem::RegisterEntity(entity);

    if (NeedToBeTracked(entity))
    {
        WatchAll(entity);
    }
}

void SnapshotSystemBase::UnregisterEntity(Entity* entity)
{
    SceneSystem::UnregisterEntity(entity);
    UnwatchAll(entity);
}

void SnapshotSystemBase::RegisterComponent(Entity* entity, Component* component)
{
    SceneSystem::RegisterComponent(entity, component);

    if (NeedToBeTracked(entity))
    {
        WatchAll(entity);
    }
}

void SnapshotSystemBase::UnregisterComponent(Entity* entity, Component* component)
{
    SceneSystem::UnregisterComponent(entity, component);
    Unwatch(entity, component);

    if (!NeedToBeTracked(entity))
    {
        UnwatchAll(entity);
    }
}

void SnapshotSystemBase::RegisterSingleComponent(Component* component)
{
    Watch(nullptr, component);
}

void SnapshotSystemBase::UnregisterSingleComponent(Component* component)
{
    Unwatch(nullptr, component);
}

void SnapshotSystemBase::PrepareForRemove()
{
    watchPoints.clear();
}

bool SnapshotSystemBase::NeedToBeTracked(Entity* entity)
{
    if (nullptr != entity)
    {
        return (nullptr != entity->GetComponent<NetworkReplicationComponent>());
    }

    return true;
}

bool SnapshotSystemBase::NeedToBeTracked(Component* component)
{
    DVASSERT(nullptr != component);

    const ReflectedType* compRefType = ReflectedTypeDB::GetByPointer(component);

    DVASSERT(nullptr != compRefType);
    if (nullptr == compRefType)
        return false;

    const M::Replicable* compReplicableMeta = compRefType->GetMeta<M::Replicable>();
    return (nullptr != compReplicableMeta) && (compReplicableMeta->privacy >= Metas::Privacy::PRIVATE);
}

SnapshotEntity* SnapshotSystemBase::GetSnapshotEntity(Entity* entity, GetBranchPolicy policy)
{
    SnapshotEntity* ret = nullptr;
    NetworkID entityId = NetworkCoreUtils::GetEntityId(entity);

    ret = snapshot.FindEntity(entityId);

    if (nullptr == ret && policy == GetBranchPolicy::Create)
    {
        Entity* parent = entity->GetParent();
        NetworkID parentId = NetworkCoreUtils::GetEntityId(parent);

        ret = snapshot.AddEntity(entity, parentId);
    }

    if (nullptr != ret)
    {
        DVASSERT(ret->entity == entity);
    }

    return ret;
}

void SnapshotSystemBase::WatchAll(Entity* entity)
{
    DVASSERT(NeedToBeTracked(entity));

    uint32 componentSz = entity->GetComponentCount();
    for (uint32_t i = 0; i < componentSz; ++i)
    {
        Component* component = entity->GetComponentByIndex(i);
        Watch(entity, component);
    }
}

void SnapshotSystemBase::Watch(Entity* entity, Component* component)
{
    // get appropriate entity snapshot
    SnapshotEntity* snapshotEntity = GetSnapshotEntity(entity, GetBranchPolicy::Create);

    if (NeedToBeTracked(component))
    {
        uint32 componentId = ComponentUtils::GetRuntimeIndex(component);
        uint32 componentIndex = GetComponentIndex(entity, component);
        SnapshotComponentKey componentKey(componentId, componentIndex);

        // get appropriate component snapshot
        auto it = snapshotEntity->components.find(componentKey);
        if (it == snapshotEntity->components.end())
        {
            // need to watch for new component
            // so create appropriate snapshot
            SnapshotComponent& snapshotComponent = snapshotEntity->components[componentKey];
            snapshotComponent.Fill(component);

            // add component fields for watching
            {
                ReflectedObject compObj(component);
                Reflection compRef = Reflection::Create(compObj);

                const Vector<ReflectedComponentField>& fields = SnapshotUtils::GetComponentFields(compRef);

                for (size_t i = 0; i < fields.size(); ++i)
                {
                    const ReflectedComponentField* compField = &fields[i];

                    auto it = snapshotEntity->components.find(componentKey);
                    DVASSERT(it != snapshotEntity->components.end());

                    SnapshotField* snapField = &it->second.fields[i];
                    watchPoints.emplace_back(snapField, entity, componentKey, compObj, compField);
                }
            }
        }
        else
        {
            // already watching for this component
            // just debug-check component pointer
            DVASSERT(it->second.component == component);
        }
    }
}

void SnapshotSystemBase::UnwatchAll(Entity* entity)
{
    // just remove incoming Entity and
    // don't care for entities children
    // they should be removed separately with
    // appropriate ::UnregisterEntity call
    size_t i = 0;
    while (i < watchPoints.size())
    {
        if (watchPoints[i].entity == entity)
        {
            watchPoints[i] = watchPoints.back();
            watchPoints.pop_back();
        }
        else
        {
            i++;
        }
    }

    snapshot.RemoveEntity(entity);
}

void SnapshotSystemBase::Unwatch(Entity* entity, Component* component)
{
    DVASSERT(component->GetEntity() == entity);

    uint32 componentId = ComponentUtils::GetRuntimeIndex(component);
    uint32 componentIndex = GetComponentIndex(entity, component);

    SnapshotComponentKey componentKey(componentId, componentIndex);

    size_t i = 0;
    while (i < watchPoints.size())
    {
        if (watchPoints[i].entity == entity && watchPoints[i].componentKey == componentKey)
        {
            watchPoints[i] = watchPoints.back();
            watchPoints.pop_back();
        }
        else
        {
            i++;
        }
    }

    SnapshotEntity* se = GetSnapshotEntity(entity, GetBranchPolicy::Get);
    if (nullptr != se)
    {
        se->components.erase(componentKey);
    }
}

Any SnapshotSystemBase::ApplyQuantization(const Any& value, uint32 compression, float32 precision)
{
    static std::array<uint8, 64> tmp;

    Any ret = value;
    if (compression != 0)
    {
        const CompressorInterface* compressor = CompressionUtils::GetTypeCompressor(value);

        BitWriter bw(tmp.data(), tmp.size());
        compressor->CompressFull(ret, compression, precision, bw);
        bw.Flush();

        BitReader br(tmp.data(), tmp.size());
        compressor->DecompressFull(ret, compression, br);
    }

    return ret;
}

uint32 SnapshotSystemBase::GetComponentIndex(Entity* entity, Component* component)
{
    uint32 index = 0;

    if (entity != nullptr)
    {
        const Type* componentType = component->GetType();

        for (;; ++index)
        {
            Component* c = entity->GetComponent(componentType, index);
            if (c == component || c == nullptr)
            {
                break;
            }
        }
    }

    return index;
}

} // namespace DAVA
