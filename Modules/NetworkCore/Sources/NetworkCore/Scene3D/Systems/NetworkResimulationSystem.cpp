#include "NetworkResimulationSystem.h"

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Systems/NetworkTransformFromNetToLocalSystem.h>
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkPredictionSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkResimulationSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Entity/SystemManager.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Scene3D/Systems/TransformSystem.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkResimulationSystem)
{
    ReflectionRegistrator<NetworkResimulationSystem>::Begin()[M::Tags("network", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkResimulationSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 12.2f)]
    .End();
}

NetworkResimulationSystem::NetworkResimulationSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkPredictComponent, NetworkReplicationComponent>())
{
    networkTimeSingleComponent = scene->GetSingleComponentForRead<NetworkTimeSingleComponent>(this);
    predictionSingleComponent = scene->GetSingleComponentForRead<NetworkPredictionSingleComponent>(this);
    snapshotSingleComponent = scene->GetSingleComponentForWrite<SnapshotSingleComponent>(this);
    networkResimulationSingleComponent = scene->GetSingleComponentForWrite<NetworkResimulationSingleComponent>(this);

    // Collect already added systems.
    for (SceneSystem* system : scene->GetSystems())
    {
        OnSystemAdded(system);
    }

    scene->systemAdded.Connect(this, &NetworkResimulationSystem::OnSystemAdded);
    scene->systemRemoved.Connect(this, &NetworkResimulationSystem::OnSystemRemoved);
}

NetworkResimulationSystem::~NetworkResimulationSystem()
{
    GetScene()->systemAdded.Disconnect(this);
    GetScene()->systemRemoved.Disconnect(this);
}

void NetworkResimulationSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkResimulationSystem::ProcessFixed");

    DVASSERT(IsClient(this));

    if (!predictionSingleComponent->mispredictedEntities.empty())
    {
        resimulationsCount += predictionSingleComponent->mispredictedEntities.size();

        const uint32 maxFrameId = networkTimeSingleComponent->GetFrameId();

        // TODO: change `predictionSingleComponent->mispredictedEntities` to right data structure.
        Map<uint32, Vector<Entity*>> resimulationFrameIdToEntities;

        for (const auto& p : predictionSingleComponent->mispredictedEntities)
        {
            resimulationFrameIdToEntities[p.second.frameId].push_back(p.first);
        }

        const auto& methods = GetEngineContext()->systemManager->GetFixedProcessMethods();

        const auto networkTransformFromNetToLocalSystemMethod = std::find_if(methods.begin(), methods.end(), [](const SystemManager::SceneProcessInfo& x) {
            return x.systemType->Is<NetworkTransformFromNetToLocalSystem>();
        });

        DVASSERT(networkTransformFromNetToLocalSystemMethod != methods.end());

        // There are systems whose simulation results can be fetched with delay
        // E.g. physics system starts simulation at the end of a frame and fetches results at the beginning of the next frame
        //
        // Previous implementation had fundamental problems when working with such systems. It worked using these rules:
        // - Each snapshot represented state at the end of a frame N
        // - When detecting mis-prediction on frame N, it rolled back to N and started resimulation on interval [N + 1; current frame]
        // Thus, if a system runs simulation at the end of a frame and fetches results at the beginning of the next frame,
        // and mis-prediction is detected, we roll back to the end of a frame N but we cannot run correct simulation at the end of a frame N as we did on a server,
        // since input has not been processed on a client by other systems, leading to possible incorrect simulation results
        //
        // In order to support resimulation of such systems, new implementation follows these rules:
        //  - Snapshots represent scene states at the beginning of a frame N
        //  - When detecting mis-prediction on frame N, we roll back to beginning of frame N and resimulate on interval [N; current frame)
        //  - Resimulation starts from process of NetworkTransformNetToLocalSystem,
        //    since this is the moment we have all the results from previous frame (i.e. all systems fetched their results)

        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "#> Resimulate\n");

        BaseSimulationSystem::ReSimulationOn();

        const uint32 resimulationStartFrameId = resimulationFrameIdToEntities.begin()->first;

        networkResimulationSingleComponent->SetResimulationFrameId(resimulationStartFrameId);

        for (const auto& p : resimulationSystems)
        {
            ISimulationSystem* system = dynamic_cast<ISimulationSystem*>(p.second);
            system->ReSimulationStart();
        }

        EntitiesManager* entitiesManager = GetScene()->GetEntitiesManager();

        /*                                  HERE BE DRAGONS.
            Now entities groups will be cleared and filled with entities that were mispredicted.
            Callbacks (while refilling) will NOT be emitted, since mispredicted entities have been processed already (theoretically).
            But if new entity will be added during resimulation, callbacks WILL be emitted.
        */

        entitiesManager->DetachGroups();

        Logger::Info("Resimulation from frame %u to frame %u", resimulationStartFrameId, maxFrameId);

        for (uint32 frameId = resimulationStartFrameId; frameId < maxFrameId; ++frameId)
        {
            networkResimulationSingleComponent->SetResimulationFrameId(frameId);

            const Vector<Entity*>& entities = resimulationFrameIdToEntities[frameId];

            if (!entities.empty())
            {
                Snapshot* serverSnapshot = snapshotSingleComponent->GetServerSnapshot(frameId);

                DVASSERT(nullptr != serverSnapshot);

                for (Entity* entity : entities)
                {
                    DVASSERT(entity != nullptr);

                    NetworkID entityId = NetworkCoreUtils::GetEntityId(entity);

                    LOG_SNAPSHOT_SYSTEM(SnapshotUtils::Log() << "## Rolling back Entity " << entityId << " to " << frameId << "\n");

                    // Revert entity to the specified frameId state.
                    SnapshotUtils::ApplySnapshot(serverSnapshot, entityId, entity);

                    entitiesManager->RegisterDetachedEntity(entity);
                }

                entitiesManager->UpdateCaches();
            }

            auto InvokeIfPartOfResimulation = [this, entitiesManager](const SystemManager::SceneProcessInfo& processInfo) {
                auto it = resimulationSystems.find(processInfo.systemType);
                if (it != resimulationSystems.end())
                {
                    SceneSystem* system = it->second;
                    processInfo.method->InvokeWithCast(system, NetworkTimeSingleComponent::FrameDurationS);
                    entitiesManager->UpdateCaches();
                }
            };

            // Invoke resimulation systems processes from NetworkTransformFromNetToLocalSystem (including) to the end.
            for (auto method = networkTransformFromNetToLocalSystemMethod; method != methods.end(); ++method)
            {
                InvokeIfPartOfResimulation(*method);
            }

            GetScene()->ClearFixedProcessesSingleComponents();

            GetScene()->GetSystem<TransformSystem>()->Process(0);
            entitiesManager->UpdateCaches();

            GetScene()->ClearAllProcessesSingleComponents();

            // Invoke resimulation systems processes from begin to the NetworkTransformFromNetToLocalSystem (excluding).
            for (auto method = methods.begin(); method != networkTransformFromNetToLocalSystemMethod; ++method)
            {
                InvokeIfPartOfResimulation(*method);
            }

            LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "##> Simulation is done for frame " << frameId << std::endl);
        }

        /*
            Restore entities groups.
            All NEW entities that were added (and not removed) during resimulation will be merged to original groups.
        */
        entitiesManager->RestoreGroups();

        for (const auto& p : resimulationSystems)
        {
            ISimulationSystem* system = dynamic_cast<ISimulationSystem*>(p.second);
            system->ReSimulationEnd();
        }

        BaseSimulationSystem::ReSimulationOff();

        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "#< Resimulate Done\n");
    }
}

void NetworkResimulationSystem::PrepareForRemove()
{
}

uint32 NetworkResimulationSystem::GetResimulationsCount() const
{
    return resimulationsCount;
}

void NetworkResimulationSystem::OnSystemAdded(SceneSystem* system)
{
    if (dynamic_cast<ISimulationSystem*>(system) != nullptr)
    {
        const Type* type = ReflectedTypeDB::GetByPointer(system)->GetType();
        DVASSERT(resimulationSystems.count(type) == 0);
        resimulationSystems[type] = system;
    }
}

void NetworkResimulationSystem::OnSystemRemoved(SceneSystem* system)
{
    if (dynamic_cast<ISimulationSystem*>(system) != nullptr)
    {
        const Type* type = ReflectedTypeDB::GetByPointer(system)->GetType();
        DVASSERT(resimulationSystems.count(type) != 0);
        resimulationSystems.erase(type);
    }
}
}