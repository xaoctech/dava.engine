
#include "NetworkCore/Scene3D/Systems/NetworkReplicationSystem2.h"
#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkDeltaSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"

#include "NetworkCore/NetworkCoreUtils.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedTypeDB.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkReplicationSystem2)
{
    ReflectionRegistrator<NetworkReplicationSystem2>::Begin()[M::Tags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkReplicationSystem2::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 8.0f)]
    .End();
}

NetworkReplicationSystem2::NetworkReplicationSystem2(Scene* scene)
    : SceneSystem(scene, 0)
{
    networkEntities = scene->GetSingletonComponent<NetworkEntitiesSingleComponent>();
    networkReplicationSingleComponent = scene->GetSingletonComponent<NetworkReplicationSingleComponent>();
    networkDeltaSingleComponent = scene->GetSingletonComponent<NetworkDeltaSingleComponent>();
    snapshotSingleComponent = scene->GetSingletonComponent<SnapshotSingleComponent>();
    networkTimeSingleComponent = scene->GetSingletonComponent<NetworkTimeSingleComponent>();
}

void NetworkReplicationSystem2::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkReplicationSystem2::ProcessFixed");

    NetworkDeltaSingleComponent::Deltas& deltas = networkDeltaSingleComponent->deltas;
    if (!deltas.empty())
    {
        pendingAddEntityOrdered.clear();
        pendingAddComponent.clear();

        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "#> ServerDiff\n");

        // Parse all accumulated Diffs.
        {
            DAVA_PROFILER_CPU_SCOPE("NetworkReplicationSystem2::ApplyDiff");

            UnorderedMap<NetworkID, uint32> entityLastFrames;

            auto it = deltas.rbegin();
            auto end = deltas.rend();
            for (; it != end; ++it)
            {
                NetworkDeltaSingleComponent::Delta& delta = *it;

                DVASSERT(delta.status == NetworkDeltaSingleComponent::Delta::Status::PENDING);

                LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "##> GotServerDiff for entity " << delta.netEntityId << " | frame(" << delta.baseFrameId << " -> " << delta.frameId << ")\n");

                uint32& entityLastFrame = entityLastFrames[delta.netEntityId];
                bool applyDone = false;
                if (entityLastFrame < delta.frameId)
                {
                    SnapshotSingleComponent::ApplyDiffParams params;
                    params.entityId = delta.netEntityId;
                    params.frameId = delta.frameId;
                    params.frameIdBase = delta.baseFrameId;
                    params.buff = delta.srcBuff;
                    params.buffSize = delta.srcSize;

                    uint32 frameId = params.frameId;
                    applyDone = snapshotSingleComponent->ApplyServerDiff(params, [this, frameId](SnapshotApplyParam& param) {
                        ApplyDiffCallback(frameId, param);
                    });

                    if (applyDone)
                    {
                        delta.status = NetworkDeltaSingleComponent::Delta::Status::APPLIED;
                        entityLastFrame = delta.frameId;

                        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "##< Ok\n");
                    }
                    else
                    {
                        delta.status = NetworkDeltaSingleComponent::Delta::Status::SKIPPED;
                        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "##< Failed\n");
                    }
                }
                else
                {
                    delta.status = NetworkDeltaSingleComponent::Delta::Status::SKIPPED;
                    LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "##< Skipped as too old\n");
                }
            }
        }

        // Add pending components, but apply snapshot
        // for snapshot changes before adding them.
        // We are forced to do this to prevent cases when
        // component was created and added,
        // but hasn't applied fields from snapshot.
        if (pendingAddComponent.size() > 0)
        {
            LOG_SNAPSHOT_SYSTEM(SnapshotUtils::Log() << "##> AddingPendingComponents\n");

            auto it = pendingAddComponent.begin();
            auto end = pendingAddComponent.end();
            for (; it != end; ++it)
            {
                Entity* entity = it->first;

                auto it2 = it->second.components.begin();
                auto end2 = it->second.components.end();
                for (; it2 != end2; ++it2)
                {
                    SnapshotComponentKey componentKey = it2->first;
                    Component* component = it2->second;

                    NetworkID entityId = NetworkCoreUtils::GetEntityId(entity);
                    DVASSERT(entityId != NetworkID::INVALID);
                    DVASSERT(entityId != NetworkID::SCENE_ID);

                    uint32 frameId = networkReplicationSingleComponent->replicationInfo.at(entityId).frameIdLastChange;
                    Snapshot* snapshot = snapshotSingleComponent->GetServerSnapshot(frameId);
                    SnapshotUtils::ApplySnapshot(snapshot, entityId, componentKey, component);

                    entity->AddComponent(component);
                }
            }

            LOG_SNAPSHOT_SYSTEM(SnapshotUtils::Log() << "##< AddingPendingComponents Done" << std::endl);
        }

        // Apply recent snapshots for all touched entities.
        if (networkReplicationSingleComponent->replicationInfo.size() > 0)
        {
            DAVA_PROFILER_CPU_SCOPE("NetworkReplicationSystem2::ApplyReplication");
            LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "##> ApplyingChanges\n");

            uint32 currentFrameId = networkTimeSingleComponent->GetFrameId();
            for (auto& info : networkReplicationSingleComponent->replicationInfo)
            {
                // Server snapshots can some times be ahead of the current
                // client frameId. But we can apply only thous server entities
                // that are below currentFrameId.
                if (currentFrameId >= info.second.frameIdLastTouch)
                {
                    info.second.frameIdServer = info.second.frameIdLastTouch;

                    Entity* entity = networkEntities->FindByID(info.first);
                    uint32 frameId = info.second.frameIdLastChange;

                    // Apply only recently touched entities and
                    // remember frameId on witch that entity was applied.
                    if (frameId > info.second.frameIdLastApply)
                    {
                        info.second.frameIdLastApply = frameId;

                        Snapshot* snapshot = snapshotSingleComponent->GetServerSnapshot(frameId);
                        if (snapshot != nullptr)
                        {
                            ApplySnapshotWithoutPrediction(entity, snapshot);
                        }
                    }
                }
            }

            LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "##< ApplyingChanges Done" << std::endl);
        }

        // Add pending entities.
        // We are forced to add them later to prevent cases when
        // we are adding newly create entity without its components,
        // that should be applied from snapshot.
        if (pendingAddEntityOrdered.size() > 0)
        {
            LOG_SNAPSHOT_SYSTEM(SnapshotUtils::Log() << "##> AddingPendingEntities\n");

            for (auto& toAdd : pendingAddEntityOrdered)
            {
                Entity* entity = toAdd.entity;
                Entity* parent = toAdd.parent;

                parent->AddNode(entity);

                LOG_SNAPSHOT_SYSTEM(SnapshotUtils::Log() << "| Entity " << NetworkCoreUtils::GetEntityId(entity) << "\n");
            }

            LOG_SNAPSHOT_SYSTEM(SnapshotUtils::Log() << "##< AddingPendingEntities Done" << std::endl);
        }

        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "#< ServerDiff Done" << std::endl);
    }
}

void NetworkReplicationSystem2::PrepareForRemove()
{
}

void NetworkReplicationSystem2::ApplyDiffCallback(uint32 frameId, SnapshotApplyParam& param)
{
    switch (param.cmd)
    {
    case SnapshotApplyCommand::ENTITY_ADDED:
    {
        NetworkID entityId = param.entityParam.entityId;
        DVASSERT(entityId != NetworkID::INVALID);

        Entity* entity = networkEntities->FindByID(entityId);
        Entity* parent = networkEntities->FindByID(param.entityParam.parentId);

        DVASSERT(nullptr != parent);

        if (nullptr == entity)
        {
            NetworkReplicationComponent* netRepComp = new NetworkReplicationComponent();
            netRepComp->SetNetworkID(entityId);

            entity = new Entity();
            entity->AddComponent(netRepComp);

            networkEntities->RegisterEntity(entityId, entity);

            // Delay entities addition. They will be added later,
            // when all their components are created
            AddPendingEntity(entity, parent);
        }
        else
        {
            Entity* alreadyAssignedParent = GetPendingEntityParent(entity);
            if (nullptr != alreadyAssignedParent)
            {
                // check that pending entity has appropriate parent
                DVASSERT(alreadyAssignedParent == parent);
            }
            else
            {
                // check that already created entity has appropriate parent
                DVASSERT(entity == GetScene() || entity->GetParent() == parent);
            }
        }

        // return entity by setting output parameter
        DVASSERT(entity != nullptr);
        param.entityParam.outEntity = entity;

        // update info about added entity
        networkReplicationSingleComponent->replicationInfo.insert({ entityId, NetworkReplicationSingleComponent::EntityReplicationInfo() });
        UpdateReplicationInfo(entityId, frameId, true);

        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "|- Entity " << entityId << " will be added to " << param.entityParam.parentId << "\n");
    }
    break;
    case SnapshotApplyCommand::ENTITY_REMOVED:
    {
        NetworkID entityId = param.entityParam.entityId;
        DVASSERT(entityId != NetworkID::INVALID);
        DVASSERT(entityId != NetworkID::SCENE_ID);

        Entity* entity = networkEntities->FindByID(entityId);
        if (nullptr != entity)
        {
            Entity* parent = entity->GetParent();
            if (nullptr != parent)
            {
                parent->RemoveNode(entity);
            }

            RemovePendingEntity(entity);
            RemovePendingComponents(entity);

            // erase info about removed entity
            networkReplicationSingleComponent->replicationInfo.erase(entityId);
        }

        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "|- Entity " << entityId << " removed from " << param.entityParam.parentId << "\n");
    }
    break;
    case SnapshotApplyCommand::ENTITY_TOUCHED:
    {
        NetworkID entityId = param.entityParam.entityId;
        DVASSERT(entityId != NetworkID::INVALID);

        Entity* entity = networkEntities->FindByID(entityId);
        if (nullptr != entity && entityId != NetworkID::SCENE_ID)
        {
            // update info about touched entity
            UpdateReplicationInfo(entityId, frameId, false);
        }

        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "|- Entity " << entityId << " touched\n");
    }
    break;
    case SnapshotApplyCommand::COMPONENT_ADDED:
    {
        NetworkID entityId = param.componentParam.entityId;
        Entity* entity = networkEntities->FindByID(entityId);
        if (nullptr != entity)
        {
            Component* component = nullptr;

            const Type* componentType = GetEngineContext()->componentManager->GetSceneComponentType(param.componentParam.componentKey.id);
            DVASSERT(nullptr != componentType);

            // singleton components
            Scene* scene = GetScene();
            if (entity == scene)
            {
                // check if it isn't created yet
                component = scene->GetSingletonComponent(componentType);
            }
            // regular component
            else
            {
                // check if it isn't created yet
                component = entity->GetComponent(componentType);
                if (nullptr == component)
                {
                    component = ComponentUtils::Create(componentType);

                    // Delay component addition. It will be added later,
                    // when their fields are applied from snapshot.
                    AddPendingComponent(entity, component, param.componentParam.componentKey);
                }
            }

            // return component by setting output parameter
            DVASSERT(component != nullptr);
            param.componentParam.outComponent = component;

            // update info about entity, making it changed
            UpdateReplicationInfo(entityId, frameId, true);

            LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "|- Component " << param.componentParam.componentKey
                                                             << " [" << componentType->GetName() << "] will be added to entity " << entityId << "\n");
        }
    }
    break;
    case SnapshotApplyCommand::COMPONENT_REMOVED:
    {
        NetworkID entityId = param.componentParam.entityId;
        Entity* entity = networkEntities->FindByID(entityId);
        if (nullptr != entity)
        {
            const Type* componentType = GetEngineContext()->componentManager->GetSceneComponentType(param.componentParam.componentKey.id);
            DVASSERT(nullptr != componentType);

            // singleton components
            Scene* scene = GetScene();
            if (entity == scene)
            {
                DVASSERT(false, "Single components haven't be removed");
                Component* component = scene->GetSingletonComponent(componentType);
            }
            // regular component
            else
            {
                Component* component = entity->GetComponent(componentType);
                if (nullptr != component)
                {
                    entity->RemoveComponent(component);
                }
            }

            RemovePendingComponent(entity, param.componentParam.componentKey);
            UpdateReplicationInfo(entityId, frameId, false);

            LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "|- Component " << param.componentParam.componentKey
                                                             << " [" << componentType->GetName() << "] removed from entity " << entityId << "\n");
        }
    }
    break;
    case SnapshotApplyCommand::COMPONENT_CHANGED:
    {
        NetworkID entityId = param.componentParam.entityId;
        Entity* entity = networkEntities->FindByID(entityId);
        if (nullptr != entity)
        {
            // update info about entity, making it changed
            UpdateReplicationInfo(entityId, frameId, true);
        }

        const Type* componentType = GetEngineContext()->componentManager->GetSceneComponentType(param.componentParam.componentKey.id);

        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "|- Component " << param.componentParam.componentKey
                                                         << " [" << componentType->GetName() << "] updated for entity " << entityId << "\n");
    }
    break;
    }
}

void NetworkReplicationSystem2::ApplySnapshotWithoutPrediction(Entity* entity, Snapshot* snapshot)
{
    NetworkPredictComponent* npc = entity->GetComponent<NetworkPredictComponent>();

    NetworkID entityId = NetworkCoreUtils::GetEntityId(entity);
    uint32 frameId = snapshot->frameId;

    const Map<SnapshotComponentKey, Component*>* alreadyAppliedByAddStep = nullptr;

    // remember link to components that were already added
    // into specified entity on current process loop
    auto it = pendingAddComponent.find(entity);
    if (it != pendingAddComponent.end())
    {
        alreadyAppliedByAddStep = &it->second.components;
    }

    // define predicate to skip some components,
    // when snapshot will be applied below
    auto pred = [npc, frameId, alreadyAppliedByAddStep](SnapshotComponentKey componentKey, Component* component) -> bool
    {
        if (nullptr != alreadyAppliedByAddStep && alreadyAppliedByAddStep->find(componentKey) != alreadyAppliedByAddStep->end())
        {
            LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "|- Component " << componentKey << ", skip as already applied by add step\n");
            return false;
        }
        else if (nullptr != npc && npc->IsPredictedComponent(component))
        {
            LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "|- Component " << componentKey << ", skip as predicted\n");
            return false;
        }

        return true;
    };

    LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "###> ApplyEntity " << entityId << " | serverFrame " << frameId << "\n");

    // apply snapshot with defined predicate
    SnapshotUtils::ApplySnapshot(snapshot, entityId, entity, pred);

    LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "###< ApplyEntity Done" << std::endl);
}

void NetworkReplicationSystem2::UpdateReplicationInfo(NetworkID entityId, uint32 frameId, bool isChanged)
{
    auto it = networkReplicationSingleComponent->replicationInfo.find(entityId);

    if (isChanged)
    {
        DVASSERT(it != networkReplicationSingleComponent->replicationInfo.end());
    }

    if (it != networkReplicationSingleComponent->replicationInfo.end())
    {
        if (it->second.frameIdLastTouch < frameId)
        {
            it->second.frameIdLastTouch = frameId;
        }

        if (isChanged)
        {
            if (it->second.frameIdLastChange < frameId)
            {
                it->second.frameIdLastChange = frameId;
            }
        }
    }
}

void NetworkReplicationSystem2::AddPendingEntity(Entity* entity, Entity* parent)
{
    PendingEntityParams params;

    params.entity = entity;
    params.parent = parent;

    pendingAddEntityOrdered.push_back(std::move(params));
}

void NetworkReplicationSystem2::RemovePendingEntity(Entity* entity)
{
    std::remove_if(pendingAddEntityOrdered.begin(), pendingAddEntityOrdered.end(), [entity](const PendingEntityParams& params) {
        return (entity == params.entity);
    });
}

Entity* NetworkReplicationSystem2::GetPendingEntityParent(Entity* entity)
{
    auto it = std::find_if(pendingAddEntityOrdered.begin(), pendingAddEntityOrdered.end(), [entity](const PendingEntityParams& params) {
        return (entity == params.entity);
    });

    if (it != pendingAddEntityOrdered.end())
    {
        return it->parent;
    }

    return nullptr;
}

void NetworkReplicationSystem2::AddPendingComponent(Entity* entity, Component* component, SnapshotComponentKey key)
{
    DVASSERT(pendingAddComponent[entity].components.find(key) == pendingAddComponent[entity].components.end());
    pendingAddComponent[entity].components[key] = component;
}

void NetworkReplicationSystem2::RemovePendingComponent(Entity* entity, SnapshotComponentKey key)
{
    auto it1 = pendingAddComponent.find(entity);
    if (it1 != pendingAddComponent.end())
    {
        auto it2 = it1->second.components.find(key);
        if (it2 != it1->second.components.end())
        {
            it1->second.components.erase(it2);
        }
    }
}

void NetworkReplicationSystem2::RemovePendingComponents(Entity* entity)
{
    auto it = pendingAddComponent.find(entity);
    if (it != pendingAddComponent.end())
    {
        pendingAddComponent.erase(it);
    }
}

} // namespace DAVA
