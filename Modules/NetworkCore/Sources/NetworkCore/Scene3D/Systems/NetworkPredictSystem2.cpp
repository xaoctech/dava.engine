#include "NetworkCore/Scene3D/Systems/NetworkPredictSystem2.h"

#include "NetworkCore/Scene3D/Systems/NetworkReplicationSystem2.h"
#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkPredictionSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkPredictSystem2)
{
    ReflectionRegistrator<NetworkPredictSystem2>::Begin()[M::Tags("network", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkPredictSystem2::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 10.0f)]
    .End();
}

NetworkPredictSystem2::NetworkPredictSystem2(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkPredictComponent>())
{
    snapshotSingleComponent = scene->GetSingletonComponent<SnapshotSingleComponent>();
    replicationComponent = scene->GetSingletonComponent<NetworkReplicationSingleComponent>();
    predictionComponent = scene->GetSingletonComponent<NetworkPredictionSingleComponent>();
}

void NetworkPredictSystem2::AddEntity(Entity* entity)
{
    NetworkID entityId = NetworkCoreUtils::GetEntityId(entity);
    DVASSERT(entityId != NetworkID::INVALID);

#if 0
    if (isResimulation)
    {
        // On re-simulation phase if there is a predicted entity with appropriate
        // entityId and some other entity with the same entityId is already exists
        // we should remove that first predicted entity and add new one
        // that is created on re-simulation phase.
        auto it = predictedEntityIds.find(entityId);
        if (it->second != entity)
        {
            predictedEntities.erase(it->second);
            pendingForRemove.push_back(it->second);
            SnapshotUtils::Log() << "| Removing old Entity " << entityId << " on Re-Simulation phase\n";
        }
    }
#endif

    PredictedEntityInfo info;
    if (NetworkIdSystem::IsGeneratedFromAction(entityId))
    {
        NetworkPredictComponent* netPredictComp = entity->GetComponent<NetworkPredictComponent>();
        info.creationFrameId = netPredictComp->GetFrameActionID().frameId;
    }
    predictedEntities[entity] = std::move(info);
    predictedEntityIds[entityId] = entity;
}

void NetworkPredictSystem2::RemoveEntity(Entity* entity)
{
    predictedEntities.erase(entity);
}

void NetworkPredictSystem2::ProcessFixed(float32 timeElapsed)
{
    uint32 lastClientSnapshotFrameId = snapshotSingleComponent->clientHistory[snapshotSingleComponent->clientHistoryPos].frameId;
    uint32 lastServertSnapshotFrameId = snapshotSingleComponent->serverHistory[snapshotSingleComponent->serverHistoryPos].frameId;
    if (lastServertSnapshotFrameId > lastClientSnapshotFrameId)
    {
        return;
    }

    predictionComponent->mispredictedEntities.clear();
    const NetworkReplicationSingleComponent::FullyReceivedFrames& fullyReceivedFrames = replicationComponent->fullyReceivedFrames;
    if (predictedEntities.size() > 0)
    {
        LOG_SNAPSHOT_SYSTEM(SnapshotUtils::Log() << "#> PredCheck\n");

        auto it = predictedEntities.begin();
        auto end = predictedEntities.end();
        for (; it != end; ++it)
        {
            bool existConfirmed = false;

            Entity* entity = it->first;
            PredictedEntityInfo& info = it->second;
            NetworkID entityId = NetworkCoreUtils::GetEntityId(entity);

            auto replicationInfoIt = replicationComponent->replicationInfo.find(entityId);
            const bool hasAnyReplicationInfo = (replicationInfoIt != replicationComponent->replicationInfo.end());
            if (hasAnyReplicationInfo)
            {
                uint32 lastServerFrameId = replicationInfoIt->second.frameIdServer;
                if (lastServerFrameId > info.lastExistanceFrameId)
                {
                    info.lastExistanceFrameId = lastServerFrameId;
                    existConfirmed = true;

                    Snapshot* clientSnapshot = snapshotSingleComponent->GetClientSnapshot(lastServerFrameId);
                    Snapshot* serverSnapshot = snapshotSingleComponent->GetServerSnapshot(lastServerFrameId);

                    if (nullptr != clientSnapshot && nullptr != serverSnapshot)
                    {
                        EntityMisprediction misprediction;
                        bool componentsConfirmed = ConfirmComponentValues(clientSnapshot, serverSnapshot, entityId, misprediction);
                        if (!componentsConfirmed)
                        {
                            misprediction.frameId = lastServerFrameId;
                            predictionComponent->mispredictedEntities[entity] = std::move(misprediction);

                            LOG_SNAPSHOT_SYSTEM(SnapshotUtils::Log() << "| Mis-Prediction for entity " << entityId << " on serverFrame " << lastServerFrameId << std::endl);
                        }
                    }
                }
            }

            // track ttl only for
            // generated entities
            const uint32 creationFrameId = info.creationFrameId;
            if (creationFrameId != 0)
            {
                if (existConfirmed)
                {
                    info.ttl = maxTTL;
                }
                else
                {
                    bool waitConfirmation = false;
                    if (info.ttl > 0)
                    {
                        info.ttl--;
                        waitConfirmation = true;

                        if (!hasAnyReplicationInfo)
                        {
                            const auto hasFullyFrame = [creationFrameId](const uint32 frameId)
                            {
                                return creationFrameId <= frameId;
                            };
                            auto findIt = std::find_if(fullyReceivedFrames.begin(), fullyReceivedFrames.end(), hasFullyFrame);
                            waitConfirmation = (findIt == fullyReceivedFrames.end());
                            ;
                        }
                    }

                    if (!waitConfirmation)
                    {
                        pendingForRemove.push_back(entity);
                    }
                }
            }
        }

        if (!pendingForRemove.empty())
        {
            for (size_t i = 0; i < pendingForRemove.size(); ++i)
            {
                Entity* entity = pendingForRemove[i];
                entity->GetParent()->RemoveNode(entity);
                LOG_SNAPSHOT_SYSTEM(SnapshotUtils::Log() << "## Entity " << NetworkCoreUtils::GetEntityId(entity) << " removed by TTL\n");
            }

            pendingForRemove.clear();
        }

        LOG_SNAPSHOT_SYSTEM(SnapshotUtils::Log() << "#< PredCheck Done" << std::endl);
    }
}

void NetworkPredictSystem2::PrepareForRemove()
{
}

void NetworkPredictSystem2::ReSimulationStart(Entity* entity, uint32 frameId)
{
    isResimulation = true;
}

void NetworkPredictSystem2::ReSimulationEnd(Entity* entity)
{
    isResimulation = false;
}

const ComponentMask& NetworkPredictSystem2::GetResimulationComponents() const
{
    return SceneSystem::GetRequiredComponents();
}

void NetworkPredictSystem2::Simulate(Entity* entity)
{
}

bool NetworkPredictSystem2::ConfirmComponentValues(Snapshot* clientSnapshot, Snapshot* serverSnapshot, NetworkID entityId, EntityMisprediction& misprediction)
{
    bool ret = true;

    SnapshotEntity* clientEnt = clientSnapshot->FindEntity(entityId);
    SnapshotEntity* serverEnt = serverSnapshot->FindEntity(entityId);

    if (clientEnt != nullptr && serverEnt != nullptr)
    {
        for (auto& clientC : clientEnt->components)
        {
            SnapshotComponentKey clientComponentKey = clientC.first;
            SnapshotComponent* clientComp = &clientC.second;

            auto it = serverEnt->components.find(clientComponentKey);
            if (it != serverEnt->components.end())
            {
                SnapshotComponent* serverComp = &it->second;

                if (ret && !SnapshotUtils::TestSnapshotComponentsEqual(clientComp, serverComp))
                {
                    misprediction.components.push_back(clientComponentKey);
                    ret = false;
                }
            }
        }
    }

    return ret;
}
} // namespace DAVA
