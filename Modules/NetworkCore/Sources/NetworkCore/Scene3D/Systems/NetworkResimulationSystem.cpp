#include "NetworkResimulationSystem.h"

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Systems/NetworkTransformFromNetToLocalSystem.h>
#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkPredictionSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkResimulationSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Entity/SystemManager.h>
#include <Logger/Logger.h>
#include <Render/Highlevel/RenderObject.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Scene3D/Systems/TransformSystem.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkResimulationSystem)
{
    ReflectionRegistrator<NetworkResimulationSystem>::Begin()[M::Tags("network", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkResimulationSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 13.3f)]
    .Method("CollectBbHistory", &NetworkResimulationSystem::CollectBbHistory)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 1000.f)]
    .End();
}

NetworkResimulationSystem::NetworkResimulationSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkPredictComponent, NetworkReplicationComponent>())
    , predictictedEntities(scene->AquireEntityGroup<NetworkPredictComponent, NetworkReplicationComponent>())
{
    networkTimeSingleComponent = scene->GetSingleComponentForRead<NetworkTimeSingleComponent>(this);
    predictionSingleComponent = scene->GetSingleComponentForRead<NetworkPredictionSingleComponent>(this);
    networkEntitiesSingleComponent = scene->GetSingleComponentForRead<NetworkEntitiesSingleComponent>(this);
    snapshotSingleComponent = scene->GetSingleComponentForWrite<SnapshotSingleComponent>(this);
    networkResimulationSingleComponent = scene->GetSingleComponentForWrite<NetworkResimulationSingleComponent>(this);

    networkResimulationSingleComponent->RegisterEngineVariables();

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

void NetworkResimulationSystem::CollectBbHistory(float32)
{
    auto& bbResimulation = networkResimulationSingleComponent->boundingBoxResimulation;

    if (!bbResimulation.enabled)
    {
        bbResimulation.currentHistorySize = 0;
        return;
    }

    DAVA_PROFILER_CPU_SCOPE("NetworkResimulationSystem::CollectBbHistory");

    DVASSERT(bbResimulation.history.size() == bbResimulation.HistorySize);

    const uint32 frameId = networkTimeSingleComponent->GetFrameId();

    const auto& builder = bbResimulation.GetBuilder();

    // TODO: better to store bb history inside system and manage resimulation by `ResimulationComponent` & `BbResimulationComponent`.
    // But for now resimulation logic is strictly limited to network and `NetworkPredictComponent`.
    // Also, teleportation case needs to be handled. Modification is trivial:
    // -get rid of bb history (sacrifice accuracy);
    // -replace it with movement history and static bb;
    // -add `SetBreakDistance` method.

    auto& history = bbResimulation.history[frameId % bbResimulation.HistorySize];
    history.clear();

    for (Entity* entity : predictictedEntities->GetEntities())
    {
        if (entity->GetVisible())
        {
            NetworkID eId = NetworkCoreUtils::GetEntityId(entity);

            DVASSERT(eId != NetworkID::INVALID && eId != NetworkID::SCENE_ID);
            DVASSERT(history.count(eId) == 0);

            history[eId] = builder(entity, bbResimulation.inflation);
        }
    }

    bbResimulation.lastHistoryFrameId = frameId;

    if (bbResimulation.currentHistorySize < bbResimulation.HistorySize)
    {
        ++bbResimulation.currentHistorySize;
    }
}

void NetworkResimulationSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkResimulationSystem::ProcessFixed");

    DVASSERT(IsClient(this));

    networkResimulationSingleComponent->resimulatingEntities.clear();

    if (!predictionSingleComponent->mispredictedEntities.empty())
    {
        uint32 lastServerFrameId = networkTimeSingleComponent->GetLastServerFrameId();
        const uint32 maxFrameId = networkTimeSingleComponent->GetFrameId();

        if (maxFrameId <= lastServerFrameId)
        {
            Logger::Debug("Resimulation will not be performed, since maxFrameId <= lastServerFrameId.");
            return;
        }

        UpdateListOfResimulatingEntities();

        mispredictedEntitiesCount += static_cast<uint32>(predictionSingleComponent->mispredictedEntities.size());

        const auto& resimulatingEntities = networkResimulationSingleComponent->GetResimulatingEntities();

        resimulatedEntitiesCount += static_cast<uint32>(resimulatingEntities.size());

        Map<uint32 /* frameId */, Vector<Entity*>> resimulationFrameIdToEntities;

        for (const auto& p : resimulatingEntities)
        {
            resimulationFrameIdToEntities[p.second].push_back(p.first);
        }

        const uint32 resimulationStartFrameId = resimulationFrameIdToEntities.begin()->first;

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

        networkResimulationSingleComponent->resimulationFrameId = resimulationStartFrameId;

        for (const auto& p : resimulationSystems)
        {
            ISimulationSystem* system = dynamic_cast<ISimulationSystem*>(p.second);
            system->ReSimulationStart();
        }

        EntitiesManager* entitiesManager = GetScene()->GetEntitiesManager();

        /*                                  HERE BE DRAGONS.
            Now entities groups will be cleared and filled with entities that were marked for resimulation.
            Callbacks (while refilling) will NOT be emitted, since these entities have been processed already (theoretically).
            But if new entity will be added during resimulation, callbacks WILL be emitted.
        */

        entitiesManager->DetachGroups();

        Logger::Info("Resimulation from frame %u to frame %u", resimulationStartFrameId, maxFrameId);

        for (uint32 frameId = resimulationStartFrameId; frameId < maxFrameId; ++frameId)
        {
            networkResimulationSingleComponent->resimulationFrameId = resimulationStartFrameId;

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
        Logger::Info("[Resimulation] total count: %u", mispredictedEntitiesCount);

        LOG_SNAPSHOT_SYSTEM_VERBOSE(SnapshotUtils::Log() << "#< Resimulate Done\n");
    }
}

uint32 NetworkResimulationSystem::GetMispredictedEntitiesCount() const
{
    return mispredictedEntitiesCount;
}

uint32 NetworkResimulationSystem::GetResimulatedEntitiesCount() const
{
    return resimulatedEntitiesCount;
}

void NetworkResimulationSystem::UpdateListOfResimulatingEntities()
{
    DAVA_PROFILER_CPU_SCOPE("NetworkResimulationSystem::UpdateListOfResimulatingEntities");

    Vector<std::pair<Entity*, uint32>>& resimulatingEntities = networkResimulationSingleComponent->resimulatingEntities;
    UnorderedMap<Entity*, uint32> indices;

    const uint32 maxFrameId = networkTimeSingleComponent->GetFrameId();

    uint32 resimulationStartFrameId = predictionSingleComponent->mispredictedEntities.begin()->second.frameId;

    for (const auto& p : predictionSingleComponent->mispredictedEntities)
    {
        resimulatingEntities.emplace_back(p.first, p.second.frameId);
        indices[p.first] = static_cast<uint32>(resimulatingEntities.size() - 1);
        resimulationStartFrameId = Min(resimulationStartFrameId, p.second.frameId);
    }

    DVASSERT(maxFrameId >= resimulationStartFrameId);

    const auto& bbResimulation = networkResimulationSingleComponent->boundingBoxResimulation;

    if (bbResimulation.enabled)
    {
        DAVA_PROFILER_CPU_SCOPE("NetworkResimulationSystem::BoundingBoxResimulation");

        UnorderedMap<Entity*, AABBox3> boxes;

        uint32 fdiff = maxFrameId - resimulationStartFrameId;

        if (bbResimulation.currentHistorySize >= fdiff)
        {
            // Construct bounding boxes for each entity for last fdiff frames.
            for (uint32 frameId = resimulationStartFrameId; frameId < maxFrameId; ++frameId)
            {
                for (const auto& p : bbResimulation.GetHistoryForFrame(frameId))
                {
                    Entity* entity = networkEntitiesSingleComponent->FindByID(p.first);

                    if (entity != nullptr)
                    {
                        boxes[entity].AddAABBox(p.second);
                    }
                }
            }

            // TODO: in case of poor performance optimize it with sectors.
            for (size_t i = 0; i < resimulatingEntities.size(); ++i)
            {
                auto* resimulatingEntity = &resimulatingEntities[i];

                const AABBox3& box = boxes[resimulatingEntity->first];

                DVASSERT(!box.IsEmpty());

                for (const auto& p : boxes)
                {
                    if (box.IntersectsWithBox(p.second) && (!bbResimulation.customCollisionCheckerIsSet || bbResimulation.collisionChecker(resimulatingEntity->first, p.first)))
                    {
                        const auto it = indices.find(p.first);

                        if (it == indices.end())
                        {
                            uint32 frameId = resimulatingEntity->second;
                            resimulatingEntities.emplace_back(p.first, frameId);
                            indices[p.first] = static_cast<uint32>(resimulatingEntities.size() - 1);
                            resimulatingEntity = &resimulatingEntities[i];
                        }
                        else
                        {
                            uint32 min = Min(resimulatingEntity->second, resimulatingEntities[it->second].second);
                            resimulatingEntities[it->second].second = min;
                            resimulatingEntity->second = min;
                        }
                    }
                }
            }
        }
        else
        {
            Logger::Warning("Bounding box resimulation can not be performed due to insufficient current history size: fdiff: %u | curr. history: %u", fdiff, bbResimulation.currentHistorySize);
            Logger::Warning("Performing resimulation only for mispredicted entities.");
            DVASSERT(fdiff <= bbResimulation.HistorySize, "Frame diff > bounding box history size. Safe to skip.");
        }
    }
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
