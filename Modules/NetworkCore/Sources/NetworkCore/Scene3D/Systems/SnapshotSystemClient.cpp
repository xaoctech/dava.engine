#include "NetworkCore/Scene3D/Systems/SnapshotSystemClient.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/SnapshotUtils.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SnapshotSystemClient)
{
    ReflectionRegistrator<SnapshotSystemClient>::Begin()[M::Tags("client")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &SnapshotSystemClient::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 4.0f)]
    .End();
}

bool SnapshotSystemClient::NeedToBeTracked(Entity* entity)
{
    if (nullptr != entity)
    {
        if (nullptr == entity->GetComponent<NetworkPredictComponent>())
        {
            return false;
        }
    }

    return SnapshotSystemBase::NeedToBeTracked(entity);
}

bool SnapshotSystemClient::NeedToBeTracked(Component* component)
{
    Entity* entity = component->GetEntity();
    if (entity != nullptr)
    {
        NetworkPredictComponent* npc = entity->GetComponent<NetworkPredictComponent>();
        return npc->IsPredictedComponent(ReflectedTypeDB::GetByPointer(component)->GetType());
    }

    return false;
}

void SnapshotSystemClient::RegisterEntity(Entity* entity)
{
#if 0
    if (isResimulation)
    {
        // On re-simulation phase if there is a request for watching new entity
        // with appropriate entityId, but some other entity with the same entityId
        // is already watched, we should do the following:
        // Remove that watched entity and add for watching the new one
        // that is created on re-simulation phase.

        NId2 entityId = NetworkCoreUtils::GetEntityId(entity);
        SnapshotEntity* snapshotEntity = snapshot.FindEntity(entityId);
        if (snapshotEntity != nullptr)
        {
            Entity* watchedEntity = snapshotEntity->entity;
            if (watchedEntity != entity)
            {
                SnapshotSystemBase::UnwatchAll(watchedEntity);
                SnapshotUtils::Log() << "| Removing old Entity " << entityId << " from watching on Re-Simulation phase\n";
            }
        }
    }
#endif

    SnapshotSystemBase::RegisterEntity(entity);
}

void SnapshotSystemClient::RegisterComponent(Entity* entity, Component* component)
{
    const Type* compType = ReflectedTypeDB::GetByPointer(component)->GetType();
    if (compType->Is<NetworkPredictComponent>())
    {
        RegisterEntity(entity);
    }
    else
    {
        SnapshotSystemBase::RegisterComponent(entity, component);
    }
}

void SnapshotSystemClient::UnregisterComponent(Entity* entity, Component* component)
{
    const Type* compType = ReflectedTypeDB::GetByPointer(component)->GetType();
    if (compType->Is<NetworkPredictComponent>())
    {
        UnregisterEntity(entity);
    }
    else
    {
        SnapshotSystemBase::UnregisterComponent(entity, component);
    }
}

void SnapshotSystemClient::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("SnapshotSystemClient::ProcessFixed");

    UpdateSnapshot();

    uint32 curFrameId = timeSingleComponent->GetFrameId();
    Snapshot* s = snapshotSingleComponent->CreateClientSnapshot(curFrameId);

    if (nullptr != s)
    {
        *s = snapshot;
        s->frameId = curFrameId;
    }
}

void SnapshotSystemClient::ReSimulationStart(Entity* entity, uint32 frameId)
{
    isResimulation = true;
}

void SnapshotSystemClient::ReSimulationEnd(Entity* entity)
{
    isResimulation = false;
}

const ComponentMask& SnapshotSystemClient::GetResimulationComponents() const
{
    // should be empty for now
    return SceneSystem::GetRequiredComponents();
}

void SnapshotSystemClient::Simulate(Entity* entity)
{
    uint32 simulateFrameId = timeSingleComponent->GetFrameId();
    Snapshot* clientSnapshot = snapshotSingleComponent->GetClientSnapshot(simulateFrameId);

    if (nullptr != clientSnapshot)
    {
        NetworkID entityId = NetworkCoreUtils::GetEntityId(entity);
        SnapshotEntity* target = clientSnapshot->FindEntity(entityId);

        if (nullptr != target)
        {
            LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "#> ReSnap entity " << entityId << " | frame " << simulateFrameId << "\n");

            UpdateSnapshot(entityId);

            auto it = snapshot.entities.find(entityId);
            if (it != snapshot.entities.end())
            {
                *target = it->second;
            }

            LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "#< ReSnap Done\n");
        }
        else
        {
            LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "# ReSnap Failed, can't find target entity " << entityId << "\n");
        }
    }
    else
    {
        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "# ReSnap Failed, can't find target snapshot for frame" << simulateFrameId << "\n");
    }
}

} // namespace DAVA
