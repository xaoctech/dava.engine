#include "NetworkCore/Scene3D/Systems/SnapshotSystem2.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/SnapshotUtils.h"

#include <Scene3D/Scene.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SnapshotSystem2)
{
    ReflectionRegistrator<SnapshotSystem2>::Begin()[M::Tags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &SnapshotSystem2::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 8.1f)]
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(SnapshotComponent2)
{
    ReflectionRegistrator<SnapshotComponent2>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* SnapshotComponent2::Clone(Entity* toEntity)
{
    return nullptr;
}

SnapEntity* SnapshotComponent2::Get(uint32 frameId, bool create)
{
    SnapEntity* ret = nullptr;

    uint32 curFrameId = history[historyPos].frameId;
    uint32 historySize = history.size();

    // requested current frame
    if (frameId == curFrameId)
    {
        ret = &history[historyPos];
    }
    // requested past frame
    else if (frameId < curFrameId)
    {
        uint32 offset = curFrameId - frameId;
        if (offset < historySize)
        {
            uint32 p = 0;
            if (offset <= historyPos)
            {
                p = historyPos - offset;
            }
            else
            {
                p = historySize - (offset - historyPos);
            }

            // when creation requested, check if pos is free
            // and initialize it for specified frameId
            if (create && history[p].frameId == 0)
            {
                history[p].Reset(frameId);
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
                    historyPos++; // override incoming `pos`
                    if (historyPos >= historySize)
                    {
                        historyPos = 0;
                    }

                    history[historyPos].Reset();
                    DVASSERT(history[historyPos].frameId == 0);
                }

                history[historyPos].Reset(frameId);
                ret = &history[historyPos];
            }
            else
            {
                for (size_t i = 0; i < historySize; ++i)
                {
                    history[i].Reset();
                    DVASSERT(history[i].frameId == 0);
                }

                historyPos = historySize - 1;

                history[historyPos].Reset(frameId);
                ret = &history[historyPos];
            }
        }
    }

    return ret;
}

SnapshotSystem2::SnapshotSystem2(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkReplicationComponent>())
{
    for (auto& sc : scene->singleComponents)
    {
        RegisterSingleComponent(sc.second);
    }

    timeSingleComponent = scene->GetSingleComponentForRead<NetworkTimeSingleComponent>(this);
}

SnapshotSystem2::~SnapshotSystem2()
{
}

void SnapshotSystem2::ProcessFixed(float32 timeElapsed)
{
    if (!pendingEntities.empty())
    {
        for (auto& pair : pendingEntities)
        {
            Entity* entity = pair.first;
            PendingData& data = pair.second;

            if (data.needReset)
            {
                ResetEntitySnapshot(entity);
            }

            SnapshotComponent2* snapshotComponent = entity->GetComponent<SnapshotComponent2>();
            DVASSERT(nullptr != snapshotComponent);

            if (!data.entityEvents.empty())
            {
                // TODO:
                // merge events
                // ...

                snapshotComponent->snapEntity.entityEvents = std::move(data.entityEvents);
                UpdateChildrenLayoutVersion(entity);
            }

            if (!data.componentEvents.empty())
            {
                // TODO:
                // merge events
                // ...

                snapshotComponent->snapEntity.componentEvents = std::move(data.componentEvents);
                UpdateChildrenDataVersion(entity);
            }
        }

        pendingEntities.clear();
    }

    uint32 frameId = timeSingleComponent->GetFrameId();

    for (Entity* entity : snapshotedEntities)
    {
        SnapshotComponent2* snapshotComponent = entity->GetComponent<SnapshotComponent2>();
        DVASSERT(snapshotComponent != nullptr);

        bool hasChanged = false;

        SnapEntity* snapEntity = &snapshotComponent->snapEntity;
        for (SnapshotComponent2::SnapWatchPoint& wp : snapshotComponent->watchPoints)
        {
            Any value = wp.vw->GetValue(wp.object);
            if (value != wp.lastValue)
            {
                wp.lastValue = value;

                SnapComponent* snapComponent = snapEntity->components[wp.componentIndex].get();
                if (snapComponent->frameIdEnd != frameId)
                {
                    auto newSnapComponent = std::make_shared<SnapComponent>();
                    *newSnapComponent = *snapComponent;
                    newSnapComponent->frameIdBegin = frameId;
                    newSnapComponent->frameIdEnd = frameId;
                    snapEntity->components[wp.componentIndex] = newSnapComponent;

                    snapComponent = newSnapComponent.get();
                }

                snapComponent->fields[wp.fieldIndex].value = value;
                hasChanged = true;
            }
        }

        if (hasChanged)
        {
            UpdateChildrenDataVersion(entity->GetParent());
        }

        for (auto& sc : snapEntity->components)
        {
            sc->frameIdEnd = frameId;
        }

        SnapEntity* historySnapEntity = snapshotComponent->Get(frameId, true);
        if (nullptr != historySnapEntity)
        {
            historySnapEntity->frameId = frameId;
            historySnapEntity->componentsLayoutVersion = snapEntity->componentsLayoutVersion;
            historySnapEntity->childrenLayoutVersion = snapEntity->childrenLayoutVersion;
            historySnapEntity->childrenDataVersion = snapEntity->childrenDataVersion;
            historySnapEntity->componentEvents = std::move(snapEntity->componentEvents);
            historySnapEntity->entityEvents = std::move(snapEntity->entityEvents);
            historySnapEntity->components = snapEntity->components;
        }

        snapEntity->ClearEvents();
    }
}

void SnapshotSystem2::RegisterEntity(Entity* entity)
{
    SceneSystem::RegisterEntity(entity);

    if (NeedToBeTracked(entity))
    {
        AddPendingEntityReset(entity);
        AddPendingEntityEvent(entity->GetParent(), NetworkCoreUtils::GetEntityId(entity), 1);

        Watch(entity);
    }
}

void SnapshotSystem2::UnregisterEntity(Entity* entity)
{
    SceneSystem::UnregisterEntity(entity);

    if (NeedToBeTracked(entity))
    {
        AddPendingEntityEvent(entity->GetParent(), NetworkCoreUtils::GetEntityId(entity), -1);
    }

    Unwatch(entity);
}

void SnapshotSystem2::RegisterComponent(Entity* entity, Component* component)
{
    SceneSystem::RegisterComponent(entity, component);
    if (NeedToBeTracked(entity))
    {
        if (component->GetType()->Is<NetworkReplicationComponent>())
        {
            AddPendingEntityReset(entity);
            AddPendingEntityEvent(entity->GetParent(), NetworkCoreUtils::GetEntityId(entity), 1);

            Watch(entity);
        }
        else if (NeedToBeTracked(component))
        {
            AddPendingComponentEvent(entity, component, 1);
            AddPendingEntityReset(entity);
        }
    }
}

void SnapshotSystem2::UnregisterComponent(Entity* entity, Component* component)
{
    SceneSystem::UnregisterComponent(entity, component);

    if (!NeedToBeTracked(entity))
    {
        if (component->GetType()->Is<NetworkReplicationComponent>())
        {
            NetworkReplicationComponent* nrc = static_cast<NetworkReplicationComponent*>(component);
            AddPendingEntityEvent(entity->GetParent(), nrc->GetNetworkID(), -1);
        }

        Unwatch(entity);
    }
    else if (NeedToBeTracked(component))
    {
        AddPendingComponentEvent(entity, component, -1);
        AddPendingEntityReset(entity);
    }
}

void SnapshotSystem2::RegisterSingleComponent(Component* component)
{
    AddComponentSnapshot(GetScene(), component);
}

void SnapshotSystem2::UnregisterSingleComponent(Component* component)
{
    DVASSERT(false, "Should never happen");
}

void SnapshotSystem2::Watch(Entity* entity)
{
    snapshotedEntities.push_back(entity);
}

void SnapshotSystem2::Unwatch(Entity* entity)
{
    pendingEntities.erase(entity);

    // remove by swapping with last
    for (size_t i = 0; i < snapshotedEntities.size(); ++i)
    {
        if (snapshotedEntities[i] == entity)
        {
            snapshotedEntities[i] = snapshotedEntities.back();
            snapshotedEntities.pop_back();

            break;
        }
    }
}

void SnapshotSystem2::AddPendingEntityReset(Entity* entity)
{
    pendingEntities[entity].needReset = true;
}

void SnapshotSystem2::AddPendingEntityEvent(Entity* parent, NetworkID childEntityId, int32 type)
{
    if (nullptr != parent)
    {
        SnapEntityEvent e;
        e.type = type;
        e.entityId = childEntityId;

        DVASSERT(NeedToBeTracked(parent), "Replicated entity with parent that isn't replicated.");
        DVASSERT(e.entityId.IsValid());

        pendingEntities[parent].entityEvents.emplace_back(std::move(e));
    }
}

void SnapshotSystem2::AddPendingComponentEvent(Entity* entity, Component* component, int32 type)
{
    if (nullptr != entity)
    {
        SnapComponentEvent e;
        e.type = type;
        e.key.index = 0; // TODO: real index here
        e.key.id = ComponentUtils::GetRuntimeId(component->GetType());

        pendingEntities[entity].componentEvents.emplace_back(std::move(e));
    }
}

void SnapshotSystem2::ResetEntitySnapshot(Entity* entity)
{
    SnapshotComponent2* snapshotComponent = entity->GetOrCreateComponent<SnapshotComponent2>();
    NetworkReplicationComponent* nrc = entity->GetComponent<NetworkReplicationComponent>();
    ComponentMask replicationMask = nrc->GetReplicationMask();

    snapshotComponent->watchPoints.clear();
    snapshotComponent->snapEntity.ClearComponents();
    snapshotComponent->snapEntity.componentsLayoutVersion++;
    snapshotComponent->snapEntity.entityId = NetworkCoreUtils::GetEntityId(entity);

    uint32 componentIndex = 0;
    uint32 componentCount = entity->GetComponentCount();
    for (uint32 i = 0; i < componentCount; ++i)
    {
        Component* component = entity->GetComponentByIndex(i);
        AddComponentSnapshot(entity, component);
    }
}

void SnapshotSystem2::AddComponentSnapshot(Entity* entity, Component* component)
{
    SnapshotComponent2* snapshotComponent = entity->GetOrCreateComponent<SnapshotComponent2>();
    NetworkReplicationComponent* nrc = entity->GetComponent<NetworkReplicationComponent>();
    ComponentMask replicationMask = nrc->GetReplicationMask();

    const Type* componentType = component->GetType();
    if (replicationMask.IsSet(componentType))
    {
        M::Privacy privacy = M::Privacy::PRIVATE;

        ComponentMask privacyMask = nrc->GetReplicationPrivacyMask();
        if (privacyMask.IsSet(componentType))
        {
            privacy = M::Privacy::PUBLIC;
        }

        uint32 componentIndex = entity->GetComponentInnerIndex(component);
        uint32 componentId = ComponentUtils::GetRuntimeId(component->GetType());

        // need to watch for new component
        // so create appropriate snapshot
        auto snapComponent = std::make_shared<SnapComponent>();
        snapComponent->privacy = privacy;
        snapComponent->key = SnapshotComponentKey(componentId, componentIndex);

        // now add all component fields
        ReflectedObject compObj = ReflectedObject(component);
        Reflection compRef = Reflection::Create(compObj);

        DVASSERT(compRef.GetFieldsCaps().hasRangeAccess);
        DVASSERT(!compRef.GetFieldsCaps().hasDynamicStruct);

        const Vector<ReflectedComponentField>& fieldsRef = SnapshotUtils::GetComponentFields(compRef);

        // we will push_back component, so we can know
        // its index just before pushing it back
        uint32 compi = snapshotComponent->snapEntity.components.size();

        // add each field into snapshot
        // and also add them for change-watching
        snapComponent->fields.reserve(fieldsRef.size());
        for (size_t j = 0; j < fieldsRef.size(); ++j)
        {
            const M::Replicable* fieldReplicable = fieldsRef[j].replicable;
            M::Privacy fieldPrivacy = fieldReplicable->GetMergedPrivacy(privacy);

            // add field to snapshot
            {
                Any fieldValue = fieldsRef[j].valueWrapper->GetValue(compObj);

                SnapField sf;
                sf.value = std::move(fieldValue);
                sf.precision = fieldsRef[j].precision;
                sf.deltaPrecision = fieldsRef[j].deltaPrecision;
                sf.compression = fieldsRef[j].compressionScheme;
                snapComponent->fields.push_back(std::move(sf));
            }

            // add field for watching
            {
                SnapshotComponent2::SnapWatchPoint wp;
                wp.componentIndex = compi;
                wp.fieldIndex = j;
                wp.object = compObj;
                wp.vw = fieldsRef[j].valueWrapper;
                snapshotComponent->watchPoints.emplace_back(std::move(wp));
            }
        }

        snapshotComponent->snapEntity.components.emplace_back(std::move(snapComponent));
        DVASSERT(compi == (snapshotComponent->snapEntity.components.size() - 1));
    }
}

bool SnapshotSystem2::NeedToBeTracked(Entity* entity)
{
    NetworkReplicationComponent* nrc = entity->GetComponent<NetworkReplicationComponent>();
    return (nullptr != nrc);
}

bool SnapshotSystem2::NeedToBeTracked(Component* component, NetworkReplicationComponent* nrc)
{
    if (nullptr == nrc)
    {
        nrc = component->GetSibling<NetworkReplicationComponent>();
    }

    if (nullptr != nrc)
    {
        return nrc->GetReplicationMask().IsSet(component->GetType());
    }

    return false;
}

void SnapshotSystem2::UpdateChildrenLayoutVersion(Entity* entity)
{
    if (nullptr != entity)
    {
        SnapshotComponent2* snapshotComponent = entity->GetComponent<SnapshotComponent2>();
        DVASSERT(nullptr != snapshotComponent);

        snapshotComponent->snapEntity.childrenLayoutVersion++;
        snapshotComponent->snapEntity.childrenDataVersion = 0;
        UpdateChildrenLayoutVersion(entity->GetParent());
    }
}

void SnapshotSystem2::UpdateChildrenDataVersion(Entity* entity)
{
    if (nullptr != entity)
    {
        SnapshotComponent2* snapshotComponent = entity->GetComponent<SnapshotComponent2>();
        DVASSERT(nullptr != snapshotComponent);

        snapshotComponent->snapEntity.childrenDataVersion++;
        UpdateChildrenDataVersion(entity->GetParent());
    }
}

} // namespace DAVA
