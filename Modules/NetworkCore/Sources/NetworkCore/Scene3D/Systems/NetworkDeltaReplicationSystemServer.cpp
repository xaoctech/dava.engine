
#include "NetworkDeltaReplicationSystemServer.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTrafficLimitComponent.h"
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkVisibilitySingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/Snapshot.h"
#include <NetworkCore/NetworkCoreUtils.h>

#include <Scene3D/Scene.h>
#include <Scene3D/Entity.h>
#include <Logger/Logger.h>
#include <Debug/ProfilerCPU.h>

#include <Time/SystemTimer.h>
#include <algorithm>

#include <Reflection/Reflection.h>
#include <Reflection/ReflectedObject.h>
#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkDeltaReplicationSystemServer)
{
    ReflectionRegistrator<NetworkDeltaReplicationSystemServer>::Begin()[M::Tags("network", "server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkDeltaReplicationSystemServer::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 7.0f)]
    .End();
}

NetworkDeltaReplicationSystemServer::NetworkDeltaReplicationSystemServer(Scene* scene)
    : NetworkDeltaReplicationSystemBase(scene)
{
    DVASSERT(IsServer(scene));

    server = scene->GetSingletonComponent<NetworkServerSingleComponent>()->GetServer();

    server->SubscribeOnReceive(PacketParams::DELTA_REPLICATION_CHANNEL_ID,
                               OnServerReceiveCb(this, &NetworkDeltaReplicationSystemServer::OnReceiveCallback));

    server->SubscribeOnDisconnect(OnServerDisconnectCb(this, &NetworkDeltaReplicationSystemServer::OnClientDisconnect));

    snapshotSingleComponent = scene->GetSingletonComponent<SnapshotSingleComponent>();
    netVisSingleComp = scene->GetSingletonComponent<NetworkVisibilitySingleComponent>();
    netGameModeComp = scene->GetSingletonComponent<NetworkGameModeSingleComponent>();
    timeComp = scene->GetSingletonComponent<NetworkTimeSingleComponent>();
}

void NetworkDeltaReplicationSystemServer::RemoveEntity(Entity* entity)
{
    const uint32 frameId = timeComp->GetFrameId();
    const NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
    const NetworkID netEntityId = netReplComp->GetNetworkID();
    const NetworkPlayerID entPlayerId = netReplComp->GetNetworkPlayerID();
    server->Foreach([this, frameId, netEntityId, entPlayerId](const Responder& responder)
                    {
                        ResponderData& responderData = respondersData[&responder];
                        auto findIt = responderData.baseFrames.find(netEntityId);
                        if (findIt != responderData.baseFrames.end())
                        {
                            FrameRange& frameRange = findIt->second;
                            frameRange.delFrameId = frameId;
                            const NetworkPlayerID playerID = netGameModeComp->GetNetworkPlayerID(responder.GetToken());
                            responderData.removedEntities[netEntityId] = GetPrivacy(playerID, entPlayerId);
                        }
                    });
}

void NetworkDeltaReplicationSystemServer::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkDeltaReplicationSystemServer::ProcessFixed");
    server->Foreach([this](const Responder& responder)
                    {
                        ResponderData& responderData = respondersData[&responder];
                        ProcessAckPackets(responder, responderData);
                        ProcessResponder(responder, responderData);
                    });
    netVisSingleComp->ClearCache();
}

size_t
NetworkDeltaReplicationSystemServer::CreateDiff(SnapshotSingleComponent::CreateDiffParams& params)
{
    if (snapshotSingleComponent->GetServerDiff(params))
    {
        return params.outDiffSize;
    }

    return 0;
}

void NetworkDeltaReplicationSystemServer::OnReceiveCallback(const Responder& responder, const uint8* data, size_t size)
{
    if (size > 0)
    {
        DAVA_PROFILER_CPU_SCOPE("NetworkDeltaReplicationSystemServer::OnReceiveCallback");
        const uint32 offset = ackPacket.Load(data, size);
        DVASSERT(offset == size);
        ResponderData& responderData = respondersData[&responder];
        std::copy(ackPacket.sequenceIds.begin(), ackPacket.sequenceIds.end(), std::back_inserter(responderData.acks));
    }
}

void NetworkDeltaReplicationSystemServer::ProcessAckPackets(const Responder& responder, ResponderData& responderData)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkDeltaReplicationSystemServer::ProcessAckPackets");
    SeqToSentFrames& seqToSentFrames = responderData.sentFrames;
    EntityToBaseFrames& entityToBaseFrames = responderData.baseFrames;
    for (const SequenceId& sequenceId : responderData.acks)
    {
        if (0 == sequenceId)
        {
            DVASSERT(0, "Client was hacked. Ack seqId should be positive");
            continue;
        }

        const SequenceId maxSeqId = responderData.maxSeq;
        const SequenceId minSeqId = maxSeqId - MAX_SENT_COUNT;
        bool inRange = false;
        if (minSeqId < maxSeqId)
        {
            /*             minSeqId             maxSeqId
               0 _____________[====================)_____________ 2^16
                                      range
            */
            inRange = (minSeqId <= sequenceId && sequenceId < maxSeqId);
        }
        else /* SequenceId type (2^16) overrun */
        {
            /*             maxSeqId             minSeqId
                0 ============)____________________[============== 2^16
                   range-end                          range-start
            */
            inRange = (minSeqId <= sequenceId || sequenceId < maxSeqId);
        }

        if (!inRange)
        {
            Logger::Error("Ack was skipped. Enlarge MAX_SENT_COUNT");
            continue;
        }

        SequenceData& seqData = seqToSentFrames[sequenceId % MAX_SENT_COUNT];
        if (0 == seqData.frameId)
        {
            Logger::Error("Unknown seqId:%d frameId:%zu responder:%s", sequenceId, seqData.frameId, responder.GetToken().c_str());
            continue;
        }

        for (const auto& netEntityId : seqData.networkIds)
        {
            auto findIt = entityToBaseFrames.find(netEntityId);
            if (findIt != entityToBaseFrames.end())
            {
                FrameRange& frameRange = findIt->second;
                if (frameRange.baseFrameId <= seqData.frameId && seqData.frameId <= frameRange.currFrameId)
                {
                    frameRange.baseFrameId = seqData.frameId;
                }

                if (frameRange.delFrameId > 0 && frameRange.delFrameId <= frameRange.baseFrameId)
                {
                    responderData.removedEntities.erase(netEntityId);
                    entityToBaseFrames.erase(netEntityId);
                }
            }
        }

        seqData.frameId = 0;
        seqData.networkIds.clear();
    }
    responderData.acks.clear();
}

void NetworkDeltaReplicationSystemServer::OnClientDisconnect(const FastName& token)
{
    if (server->HasResponder(token))
    {
        Logger::Debug("[NetworkDeltaReplicationSystemServer::OnClientDisconnect] Wipe state token:%s", token.c_str());
        const Responder& responder = server->GetResponder(token);
        respondersData.erase(&responder);
    }
}

NetworkDeltaReplicationSystemServer::WriteResult
NetworkDeltaReplicationSystemServer::WriteEntity(NetworkID netEntityId, M::Privacy privacy, ResponderEnvironment& env)
{
    FrameRange& range = env.entityToBaseFrames[netEntityId];
    range.currFrameId = env.frameId;

    if (!tmpBlock.size)
    {
        const uint32 tmpHeadersOffset = emptyPacketHeaderSize + entHeaderSize;
        SnapshotSingleComponent::CreateDiffParams params;
        params.buff = tmpBlock.buff + tmpHeadersOffset;
        params.buffSize = tmpBlock.SIZE - tmpHeadersOffset;
        params.entityId = netEntityId;
        params.frameId = range.currFrameId;
        params.frameIdBase = range.baseFrameId;
        params.privacy = privacy;
        const size_t diffSize = CreateDiff(params);
        if (NetworkIdSystem::IsEntityIdForStaticObject(netEntityId) && diffSize == 1 && *params.buff == EMPTY_DIFF)
        { // static object has no changes
            return WriteResult::CONTINUE;
        }

#if 0
        static UnorderedMap<uint32, uint32> stats;
        static uint32 counter__ = 0;
        stats[diffSize]++;
        counter__++;
        if (counter__ > 50000)
        {
            counter__ = 0;
            for (auto& pair : stats)
            {
                Logger::Info("%u %u", pair.first, pair.second);
            }
        }
#endif

#ifdef __DAVAENGINE_DEBUG__
        CheckTrafficLimit(netEntityId, diffSize);
#endif

        DVASSERT(diffSize > 0, "Snapshot diff size bigger than BIG_BUFFER_SIZE");
        if (!diffSize)
        {
            Logger::Error("Snapshot diff size bigger than BIG_BUFFER_SIZE");
            tmpBlock.size = 0;
            return WriteResult::EXCEPTION;
        }

        range.baseFrameId = params.outFrameIdBase;
        EntityHeader entHeader;
        entHeader.netEntityId = netEntityId;
        entHeader.frameOffset = range.GetFrameCount();
        entHeader.Save(tmpBlock.buff + emptyPacketHeaderSize);
        tmpBlock.size = tmpHeadersOffset + diffSize;

        if (0 == entHeader.frameOffset || tmpBlock.size > NetworkCoreUtils::ENET_DEFAULT_MTU_UNCOMPRESSED)
        {
            DVASSERT(tmpBlock.size <= NetworkCoreUtils::ENET_DEFAULT_MTU_UNCOMPRESSED || 0 == entHeader.frameOffset,
                     "Size tmpger than MTU is possible only for full sync");
            //Logger::Debug("[WriteEntity] Full sync netEntityId:%d frameId:%d size:%d", netEntityId.networkID, range.currFrameId, tmpBlock.size);
            /*
             *  Huge and full diff are sent by reliable channel.
             *  Sooner or later client will receive full sync and depended diff together.
             */
            range.baseFrameId = range.currFrameId;
            return WriteResult::FULL_SYNC;
        }
    }

    const uint32 mtuFreeSize = NetworkCoreUtils::ENET_DEFAULT_MTU_UNCOMPRESSED - mtuBlock.size;
    const uint32 copySize = tmpBlock.size - emptyPacketHeaderSize;
    if (copySize < mtuFreeSize)
    {
        Memcpy(mtuBlock.buff + mtuBlock.size, tmpBlock.buff + emptyPacketHeaderSize, copySize);
        mtuBlock.size += copySize;
        tmpBlock.size = 0;
        SequenceData& seqData = env.seqToSentFrames[env.sequenceId % MAX_SENT_COUNT];
        if (seqData.frameId != env.frameId)
        {
            if (seqData.frameId > 0)
            {
                //                Logger::Warning("Nack seqId:%d lostFrameId:%zu currFrame:%zu", env.sequenceId, seqData.frameId, env.frameId);
                seqData.networkIds.clear();
            }
            seqData.frameId = env.frameId;
        }

        seqData.networkIds.push_back(netEntityId);
        env.pktHeader.isDirty = true;
        return WriteResult::CONTINUE;
    }

    return WriteResult::MTU_OVERFLOW;
}

void NetworkDeltaReplicationSystemServer::SendMtuBlock(ResponderEnvironment& env)
{
    env.pktHeader.frameId = env.frameId;
    env.pktHeader.sequenceId = env.sequenceId;
    PacketParams packetParams = PacketParams::Unreliable(PacketParams::DELTA_REPLICATION_CHANNEL_ID);
    env.pktHeader.Save(mtuBlock.buff);
    env.responder.Send(mtuBlock.buff, mtuBlock.size, packetParams);
    env.pktHeader.Reset();
    mtuBlock.size = env.pktHeader.GetSize();

    ++env.currPktCount;
    ++env.sequenceId;
    if (!env.sequenceId)
    {
        env.sequenceId = 1;
    }
}

void NetworkDeltaReplicationSystemServer::SendTmpBlock(ResponderEnvironment& env)
{
    const PacketHeader tmpPacketHeader = { true, nullptr, 0, env.frameId };
    PacketParams packetParams = PacketParams::Reliable(PacketParams::DELTA_REPLICATION_CHANNEL_ID);
    tmpPacketHeader.Save(tmpBlock.buff);
    env.responder.Send(tmpBlock.buff, tmpBlock.size, packetParams);
    tmpBlock.size = 0;

    ++env.currPktCount;
}

M::Privacy NetworkDeltaReplicationSystemServer::GetPrivacy(NetworkPlayerID playerId, NetworkPlayerID entityPlayerId)
{
    const bool isSelfEntity = entityPlayerId == playerId;
    if (isSelfEntity)
    {
        return M::Privacy::PRIVATE;
    }

    return M::Privacy::PUBLIC;
}

void NetworkDeltaReplicationSystemServer::ProcessResponder(const Responder& responder, ResponderData& responderData)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkDeltaReplicationSystemServer::ProcessResponder");
    const NetworkPlayerID playerID = netGameModeComp->GetNetworkPlayerID(responder.GetToken());
    const uint32 frameId = timeComp->GetFrameId();

    SequenceId& sequenceId = responderData.maxSeq;
    if (!sequenceId)
    {
        sequenceId = 1;
    }

    ResponderEnvironment env{ responder, frameId, sequenceId, responderData.baseFrames, responderData.sentFrames };
    NetworkStatisticsSingleComponent* statsComp = GetScene()->GetSingletonComponent<NetworkStatisticsSingleComponent>();
    // prob. need bool flag inside NetworkStatisticsSingleComponent here
    //if (statsComp)
    {
        uint64 tsKey = NetStatTimestamps::GetKey(frameId, playerID);
        env.pktHeader.timestamps = statsComp->RemoveTimestamps(tsKey);
        if (env.pktHeader.timestamps)
        {
            env.pktHeader.timestamps->server.sendToNet = static_cast<uint32>(SystemTimer::GetMs());
        }
    }

    mtuBlock.size = env.pktHeader.GetSize();
    const UnorderedSet<const Entity*>& addedEntities = netVisSingleComp->GetAddedEntities(playerID);
    const NetworkVisibilitySingleComponent::EntityToFrequency& entityToFrequency =
    netVisSingleComp->GetVisibleEntities(playerID);

    ProcessEntity(NetworkID::SCENE_ID, M::Privacy::PUBLIC, env);

    for (const auto& entityToFrequencyIt : entityToFrequency)
    {
        const Entity* entity = entityToFrequencyIt.first;
        if (entity->GetParent() != GetScene())
        {
            // Child entities should be replicated via root entity
            continue;
        }

        if (addedEntities.find(entity) == addedEntities.end())
        {
            const uint8 frequency = entityToFrequencyIt.second;
            if ((0 == frequency) || (frequency > 1 && (frameId + entity->GetID()) % frequency > 0))
            {
                /* Throttling. */
                continue;
            }
        }

        NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
        const NetworkID netEntityId = netReplComp->GetNetworkID();
        M::Privacy privacy = GetPrivacy(playerID, netReplComp->GetNetworkPlayerID());
        ProcessEntity(netEntityId, privacy, env);
    }

    for (const auto& it : responderData.removedEntities)
    {
        ProcessEntity(it.first, it.second, env);
    }

    if (env.pktHeader.isDirty)
    {
        env.pktHeader.allPartsCount = env.currPktCount + 1;
        SendMtuBlock(env);
    }
}

void NetworkDeltaReplicationSystemServer::ProcessEntity(NetworkID netEntityId, M::Privacy privacy, ResponderEnvironment& env)
{
    switch (WriteEntity(netEntityId, privacy, env))
    {
    case WriteResult::MTU_OVERFLOW:
    {
        SendMtuBlock(env);
        WriteResult result = WriteEntity(netEntityId, privacy, env);
        DVASSERT(WriteResult::CONTINUE == result, "Logic is corrupted");
        break;
    }

    case WriteResult::FULL_SYNC:
    {
        SendTmpBlock(env);
        break;
    }

    default:
        break;
    }
}

void NetworkDeltaReplicationSystemServer::CheckTrafficLimit(NetworkID netEntityId, uint32 diffSize)
{
    NetworkEntitiesSingleComponent* networkEntities = GetScene()->GetSingletonComponent<NetworkEntitiesSingleComponent>();
    Entity* entity = networkEntities->FindByID(netEntityId);
    if (entity)
    {
        uint32 errorThreshold = NetworkTrafficLimitComponent::DEFAULT_ERROR_THRESHOLD;
        uint32 warningThreshold = NetworkTrafficLimitComponent::DEFAULT_WARNING_THRESHOLD;
        NetworkTrafficLimitComponent* trafficLimitComponent = entity->GetComponent<NetworkTrafficLimitComponent>();
        if (trafficLimitComponent)
        {
            errorThreshold = trafficLimitComponent->errorThreshold;
            warningThreshold = trafficLimitComponent->warningThreshold;
        }

        if (diffSize > warningThreshold)
        {
            StringStream stream;
            uint32 size = 0;
            if (entity == GetScene())
            {
                for (auto& singleComp : GetScene()->singletonComponents)
                {
                    size += DumpComponent(ReflectedObject(singleComp.first), stream);
                }
                Logger::Warning("Huge size:%d scene\n%s", size, stream.str().c_str());
            }
            else
            {
                Reflection reflEntity = Reflection::Create(ReflectedObject(entity));
                Reflection reflComps = reflEntity.GetField(FastName("Components"));
                Vector<Reflection::Field> fields = reflComps.GetFields();
                for (Reflection::Field& field : fields)
                {
                    Component* comp = field.ref.GetValue().Get<Component*>();
                    size += DumpComponent(ReflectedObject(comp), stream);
                }
                Logger::Warning("Huge size:%d entityNetId:%u:%s\n%s", size, static_cast<uint32>(netEntityId),
                                entity->GetName().c_str(), stream.str().c_str());
            }

            DVASSERT(diffSize < errorThreshold, "Diff size bigger than error threshold");
        }
    }
}

uint32 NetworkDeltaReplicationSystemServer::DumpComponent(const ReflectedObject& reflectedObject, StringStream& stream)
{
    uint32 componentSize = 0;
    Reflection reflComp = Reflection::Create(reflectedObject);
    if (reflComp.GetMeta<M::Replicable>())
    {
        StringStream ss;
        for (Reflection::Field& field : reflComp.GetFields())
        {
            Reflection& ref = field.ref;
            if (ref.GetMeta<M::Replicable>())
            {
                const Reflection::FieldCaps& caps = ref.GetFieldsCaps();
                const uint32 size = ref.GetValueType()->GetSize();
                ss << "\t\t" << size << "\t" << field.key.Cast<String>() << "\n";
                componentSize += size;
            }
        }

        stream << "\t" << componentSize << "\t" << reflComp.GetValueObject().GetReflectedType()->GetPermanentName() << "\n";
        stream << ss.str();
    }

    return componentSize;
}

} //namespace DAVA
