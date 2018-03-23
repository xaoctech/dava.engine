#include "NetworkCore/Scene3D/Systems/NetworkPredictSystem.h"

#include "NetworkCore/Scene3D/Systems/NetworkReplicationSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkPredictionSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkPredictSystem)
{
    ReflectionRegistrator<NetworkPredictSystem>::Begin()[M::Tags("network", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkPredictSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 13.1f)]
    .End();
}

NetworkPredictSystem::NetworkPredictSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkPredictComponent>())
{
    snapshotSingleComponent = scene->GetSingleComponentForWrite<SnapshotSingleComponent>(this);
    replicationComponent = scene->GetSingleComponentForRead<NetworkReplicationSingleComponent>(this);
    predictionComponent = scene->GetSingleComponentForWrite<NetworkPredictionSingleComponent>(this);
}

void NetworkPredictSystem::AddEntity(Entity* entity)
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
    predictedEntities[entity] = std::move(info);
    predictedEntityIds[entityId] = entity;
}

void NetworkPredictSystem::RemoveEntity(Entity* entity)
{
    predictedEntities.erase(entity);
}

void NetworkPredictSystem::ProcessFixed(float32 timeElapsed)
{
    uint32 lastClientSnapshotFrameId = snapshotSingleComponent->clientHistory[snapshotSingleComponent->clientHistoryPos].frameId;
    uint32 lastServertSnapshotFrameId = snapshotSingleComponent->serverHistory[snapshotSingleComponent->serverHistoryPos].frameId;
    if (lastServertSnapshotFrameId > lastClientSnapshotFrameId)
    {
        return;
    }

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

            // track TTL only for "action" entities
            if (entityId.IsPlayerActionId())
            {
                // check if entity with such id was
                // received from server
                if (existConfirmed)
                {
                    info.ttl = maxTTL;
                }
                else
                {
                    // When such entity wasn't received from server it may
                    // be client-wrongly created. But wait some more time
                    // before removing it - until TTL is > 0
                    bool waitConfirmation = false;
                    if (info.ttl > 0)
                    {
                        info.ttl--;
                        waitConfirmation = true;

                        // Some times we can delete wrongly created client-side entity
                        // quicker than just waiting for TTL expiration:
                        //   if full server frame already received such as that server-frameId
                        //   is >= client-entity-creation-frameId and there was no
                        //   existence confirmation (`existConfirmed` == true) we can delete
                        //   client-entity immediately.
                        if (!hasAnyReplicationInfo)
                        {
                            uint32 creationFrameId = entityId.GetPlayerActionFrameId();
                            const auto hasFullyFrame = [creationFrameId](const uint32 frameId)
                            {
                                return creationFrameId + NetworkTimeSingleComponent::ArtificialLatency < frameId;
                            };

                            // if there is fullyReceived server frame older than
                            // frameId for client-entity we should stop waiting
                            // for that entity confirmation
                            if (std::any_of(fullyReceivedFrames.begin(), fullyReceivedFrames.end(), hasFullyFrame))
                            {
                                // such entity will be deleted
                                waitConfirmation = false;
                            }
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

void NetworkPredictSystem::PrepareForRemove()
{
}

bool NetworkPredictSystem::ConfirmComponentValues(Snapshot* clientSnapshot, Snapshot* serverSnapshot, NetworkID entityId, EntityMisprediction& misprediction)
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
