
#include "NetworkDeltaReplicationSystemServer.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTrafficLimitComponent.h"
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerConnectionsSingleComponent.h"
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
#include <FeatureManager/FeatureManager.h>

namespace DAVA
{
namespace NetworkDeltaReplicationSystemDetail
{
// BaseFrameId for each player entity per entity tries to round to nearest common and acknowledged frame (sharedFrameId).
// As a consequence, cache hit count will increase. O(N^2) versus O(N*(SnapshotHistorySize / SharedFramePeriod).
// On the other hand increasing this value leads to growth delta diff.
static const uint32 SharedFramePeriod = 16;
static_assert((SharedFramePeriod & (SharedFramePeriod - 1)) == 0, "Should be PowerOf2 or Zero");
// BaseFrameId should not be too old.
static const uint32 MaxFrameCountOverhead = SharedFramePeriod * 2;
static_assert(SharedFramePeriod < MaxFrameCountOverhead, "Should be greater than SharedFramePeriod");

bool IsEntitySharedOnFrame(NetworkID netEntityId, uint32 frameId)
{
    DAVA_IF_FEATURE(SharedFrame)
    {
        const uint32 smoothFrameId = frameId + static_cast<uint32>(netEntityId);
        const uint32 sharedFramePeriod = SharedFramePeriod;
        return (sharedFramePeriod && (smoothFrameId % sharedFramePeriod) == 0);
    }

    return false;
}

struct FrameRange
{
    uint8 GetFrameOffset(uint32 fromFrameId) const
    {
        DVASSERT(fromFrameId <= currFrameId);
        if (fromFrameId)
        {
            uint32 diff = currFrameId - fromFrameId;
            return static_cast<uint8>((diff < uint8(~0)) ? diff : 0);
        }

        return 0;
    }

    uint32 baseFrameId = 0;
    uint32 currFrameId = 0;
    uint32 delFrameId = 0;
    uint32 sharedFrameId = 0;
};

class DiffCache
{
    static const uint32 DumpStatisticSec = 30;

public:
    struct Key
    {
        M::OwnershipRelation ownership;
        NetworkID entityId;
        uint32 frameIdBase;

        bool operator==(const Key& k) const
        {
            return entityId == k.entityId && frameIdBase == k.frameIdBase && ownership == k.ownership;
        }
    };

    struct Value
    {
        const uint8* buff = nullptr;
        uint32 size = 0;
    };

    const Value* Find(const NetworkDeltaReplicationSystemServer::DiffParams& params) const
    {
        ++stats.findCount;
        const auto& findIt = index.find({ params.ownership, params.entityId, params.frameIdBase });
        if (findIt == index.end())
        {
            ++stats.missCount;
            return nullptr;
        }

        ++stats.hitCount;
        const DiffCache::Value& value = findIt->second;
        return &value;
    }

    void Set(const NetworkDeltaReplicationSystemServer::DiffParams& params)
    {
        const uint8* insPtr = buffer.Insert(params.buff, static_cast<uint32>(params.outDiffSize), 1);
        Value value = { insPtr, static_cast<uint32>(params.outDiffSize) };
        Key key = { params.ownership, params.entityId, params.frameIdBase };
        index.emplace(std::move(key), std::move(value));
    }

    void Reset(float32 timeElapsed)
    {
        buffer.Reset();
        index.clear();

        ++stats.numberOfFrames;
        stats.time += timeElapsed;
        if (DumpStatisticSec != 0 && static_cast<uint32>(stats.time) > DumpStatisticSec)
        {
            const auto getPercent = [this](uint32 val)
            {
                return static_cast<uint32>(100 * (val / static_cast<float32>(stats.findCount)));
            };

            const auto getAvgPerFrame = [this](uint32 val)
            {
                return static_cast<uint32>(val / static_cast<float32>(stats.numberOfFrames));
            };

            Logger::Debug("[DeltaDiffCache] about per frame:\n\t[Get]:%lu\n\t[Hit]:%lu (%d%%)\n\t[Mis]:%lu (%d%%)",
                          getAvgPerFrame(stats.findCount), getAvgPerFrame(stats.hitCount), getPercent(stats.hitCount),
                          getAvgPerFrame(stats.missCount), getPercent(stats.missCount));
            stats = {};
        }
    }

private:
    struct Stats
    {
        uint32 hitCount = 0;
        uint32 missCount = 0;
        uint32 findCount = 0;
        uint32 numberOfFrames = 0;
        float32 time = 0.f;
    };

    struct Hash
    {
        size_t operator()(const Key& k) const
        {
            // leave high 3 bits for ownership, middle 8 bits for frameId and fill others with entityId
            return (k.ownership << 29 | k.frameIdBase << 21 | static_cast<DAVA::uint32>(k.entityId));
        }
    };

    ElasticBuffer buffer;
    UnorderedMap<Key, Value, Hash> index;
    mutable Stats stats;
};

} //namespace NetworkDeltaReplicationSystemDetail

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkDeltaReplicationSystemServer)
{
    ReflectionRegistrator<NetworkDeltaReplicationSystemServer>::Begin()[M::SystemTags("network", "server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkDeltaReplicationSystemServer::ProcessFixed)[M::SystemProcessInfo(SPI::Group::EngineBegin, SPI::Type::Fixed, 11.0f)]
    .End();
}

NetworkDeltaReplicationSystemServer::NetworkDeltaReplicationSystemServer(Scene* scene)
    : NetworkDeltaReplicationSystemBase(scene)
    , responderDataList(MAX_NETWORK_PLAYERS_COUNT)
    , playerIdUpperBound(0)
    , diffCache(std::make_unique<NetworkDeltaReplicationSystemDetail::DiffCache>())
{
    DVASSERT(IsServer(scene));

    server = scene->GetSingleComponentForRead<NetworkServerSingleComponent>(this)->GetServer();

    netConnectionsComp = scene->GetSingleComponentForRead<NetworkServerConnectionsSingleComponent>(this);

    snapshotSingleComponent = scene->GetSingleComponentForWrite<SnapshotSingleComponent>(this);
    netGameModeComp = scene->GetSingleComponentForRead<NetworkGameModeSingleComponent>(this);
    timeComp = scene->GetSingleComponent<NetworkTimeSingleComponent>();

    playerComponentGroup = scene->AquireComponentGroup<NetworkPlayerComponent, NetworkPlayerComponent, NetworkReplicationComponent>();
    for (NetworkPlayerComponent* c : playerComponentGroup->components)
    {
        OnPlayerComponentAdded(c);
    }
    playerComponentGroup->onComponentAdded->Connect(this, &NetworkDeltaReplicationSystemServer::OnPlayerComponentAdded);
    playerComponentGroup->onComponentRemoved->Connect(this, &NetworkDeltaReplicationSystemServer::OnPlayerComponentRemoved);
}

NetworkDeltaReplicationSystemServer::~NetworkDeltaReplicationSystemServer()
{
    playerComponentGroup->onComponentAdded->Disconnect(this);
    playerComponentGroup->onComponentRemoved->Disconnect(this);
}

void NetworkDeltaReplicationSystemServer::RemoveEntity(Entity* entity)
{
    const uint32 frameId = timeComp->GetFrameId();
    const NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
    const NetworkID netEntityId = netReplComp->GetNetworkID();
    const NetworkPlayerID entPlayerId = netReplComp->GetNetworkPlayerID();

    for (NetworkPlayerID playerId = 0; playerId <= playerIdUpperBound; ++playerId)
    {
        ResponderData& data = responderDataList[playerId];
        if (!data.token.empty())
        {
            auto findIt = data.baseFrames.find(netEntityId);
            if (findIt != data.baseFrames.end())
            {
                NetworkDeltaReplicationSystemDetail::FrameRange& frameRange = findIt->second;
                frameRange.delFrameId = frameId;
                data.removedEntities[netEntityId] = GetPlayerOwnershipRelation(playerId, entPlayerId);
            }
        }
    }
}

void NetworkDeltaReplicationSystemServer::OnPlayerComponentAdded(NetworkPlayerComponent* component)
{
    NetworkReplicationComponent* replComp = component->GetEntity()->GetComponent<NetworkReplicationComponent>();
    responderDataList[replComp->GetNetworkPlayerID()].playerComponent = component;
}

void NetworkDeltaReplicationSystemServer::OnPlayerComponentRemoved(NetworkPlayerComponent* component)
{
    NetworkReplicationComponent* replComp = component->GetEntity()->GetComponent<NetworkReplicationComponent>();
    responderDataList[replComp->GetNetworkPlayerID()].playerComponent = nullptr;
}

void NetworkDeltaReplicationSystemServer::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkDeltaReplicationSystemServer::ProcessFixed");
    for (const FastName& justDisconnectedToken : netConnectionsComp->GetJustDisconnectedTokens())
    {
        OnClientDisconnect(justDisconnectedToken);
    }
    for (const FastName& justConnectedToken : netConnectionsComp->GetJustConnectedTokens())
    {
        OnClientConnect(justConnectedToken);
    }
    for (const auto& recvPacket : netConnectionsComp->GetRecvPackets(PacketParams::DELTA_REPLICATION_CHANNEL_ID))
    {
        OnReceive(recvPacket.token, recvPacket.data);
    }

    diffCache->Reset(timeElapsed);
    for (NetworkPlayerID playerId = 0; playerId <= playerIdUpperBound; ++playerId)
    {
        ResponderData& data = responderDataList[playerId];
        if (!data.token.empty())
        {
            DVASSERT(server->HasResponder(data.token));
            ProcessAckPackets(data);
            ProcessResponder(data, playerId);
        }
    }
}

size_t
NetworkDeltaReplicationSystemServer::CreateDiff(DiffParams& params)
{
    DVASSERT(params.buff != nullptr);
    DVASSERT(params.buffSize > 0);
    DVASSERT(params.frameIdBase <= params.frameId);
    // Diff will be cached if it can be used for other player. For example each diff with ownership OWNER is unique,
    // Opposite scene's diff is shared for all.
    const bool canBeCached = params.ownership != M::OwnershipRelation::OWNER || params.entityId == NetworkID::SCENE_ID;
    Snapshot* snapshotBase = snapshotSingleComponent->GetServerSnapshot(params.frameIdBase);
    if (nullptr == snapshotBase)
    {
        params.frameIdBase = 0;
    }

    if (canBeCached)
    {
        const NetworkDeltaReplicationSystemDetail::DiffCache::Value* result = diffCache->Find(params);
        if (result)
        {
            if (result->size > params.buffSize)
            {
                return 0;
            }

            Memcpy(params.buff, result->buff, result->size);

            return result->size;
        }
    }

    Snapshot* snapshot = snapshotSingleComponent->GetServerSnapshot(params.frameId);
    DVASSERT(snapshot != nullptr);
    DVASSERT(snapshot != snapshotBase);

    size_t diffSize = SnapshotUtils::CreateSnapshotDiff(snapshotBase, snapshot, params.entityId, params.ownership, params.buff, params.buffSize);
    if (diffSize > 0)
    {
        params.outDiffSize = diffSize;
        if (canBeCached)
        {
            diffCache->Set(params);
        }
    }

    return diffSize;
}

void NetworkDeltaReplicationSystemServer::OnReceive(const FastName& token, const Vector<uint8>& data)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkDeltaReplicationSystemServer::OnReceive");
    const uint32 offset = ackPacket.Load(data.data(), data.size());
    DVASSERT(offset == data.size());

    const NetworkPlayerID playerID = netGameModeComp->GetNetworkPlayerID(token);
    ResponderData& responderData = responderDataList[playerID];
    std::copy(ackPacket.sequenceIds.begin(), ackPacket.sequenceIds.end(), std::back_inserter(responderData.acks));
}

void NetworkDeltaReplicationSystemServer::ProcessAckPackets(ResponderData& responderData)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkDeltaReplicationSystemServer::ProcessAckPackets");

    using namespace NetworkDeltaReplicationSystemDetail;
    const Responder& responder = server->GetResponder(responderData.token);

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
        const SequenceId minSeqId = maxSeqId - MaxSentCount;
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
            Logger::Error("Ack was skipped. Enlarge MaxSentCount");
            continue;
        }

        SequenceData& seqData = seqToSentFrames[sequenceId % MaxSentCount];
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
                FrameRange& range = findIt->second;
                if (range.baseFrameId <= seqData.frameId && seqData.frameId <= range.currFrameId)
                {
                    if (IsEntitySharedOnFrame(netEntityId, seqData.frameId))
                    {
                        range.sharedFrameId = seqData.frameId;
                    }
                    range.baseFrameId = seqData.frameId;
                }

                if (range.delFrameId > 0 && range.delFrameId <= range.baseFrameId)
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

void NetworkDeltaReplicationSystemServer::OnClientConnect(const FastName& token)
{
    const NetworkPlayerID playerID = netGameModeComp->GetNetworkPlayerID(token);
    Logger::Debug("[NetworkDeltaReplicationSystemServer::OnClientConnect] Set %d token:%s", playerID, token.c_str());
    playerIdUpperBound = std::max(playerID, playerIdUpperBound);

    ResponderData& data = responderDataList[playerID];
    data.token = token;
}

void NetworkDeltaReplicationSystemServer::OnClientDisconnect(const FastName& token)
{
    Logger::Debug("[NetworkDeltaReplicationSystemServer::OnClientDisconnect] Wipe state token:%s", token.c_str());
    const NetworkPlayerID playerID = netGameModeComp->GetNetworkPlayerID(token);
    if (playerID)
    {
        ResponderData& data = responderDataList[playerID];
        data.token = FastName();
        data.acks.clear();
        data.baseFrames.clear();
        data.removedEntities.clear();
        data.maxSeq = 0;
    }
}

NetworkDeltaReplicationSystemServer::WriteResult
NetworkDeltaReplicationSystemServer::WriteEntity(NetworkID netEntityId, M::OwnershipRelation ownership, ResponderEnvironment& env)
{
    using namespace NetworkDeltaReplicationSystemDetail;

    FrameRange& range = env.entityToBaseFrames[netEntityId];
    range.currFrameId = env.frameId;

    if (!tmpBlock.size)
    {
        uint32 baseFrameId = range.baseFrameId;
        DAVA_IF_FEATURE(SharedFrame)
        {
            if (range.sharedFrameId && range.baseFrameId > range.sharedFrameId)
            {
                const uint32 frameCountOverhead = range.baseFrameId - range.sharedFrameId;
                if (frameCountOverhead < MaxFrameCountOverhead)
                {
                    baseFrameId = range.sharedFrameId;
                }
            }
        }

        const uint32 tmpHeadersOffset = emptyPacketHeaderSize + entHeaderSize;
        DiffParams params;
        params.buff = tmpBlock.buff + tmpHeadersOffset;
        params.buffSize = tmpBlock.Size - tmpHeadersOffset;
        params.entityId = netEntityId;
        params.frameId = range.currFrameId;
        params.frameIdBase = baseFrameId;
        params.ownership = ownership;
        const size_t diffSize = CreateDiff(params);
        // We can save net-traffic by ignoring to send touch-info for static objects.
        // If we have such entity we should just skip it and continue to the next one.
        if (netEntityId != NetworkID::SCENE_ID && netEntityId.IsStaticId())
        {
            // check if diff for static entity is empty
            if (diffSize == 1 && *params.buff == EMPTY_DIFF)
            {
                return WriteResult::CONTINUE;
            }
        }

#ifdef __DAVAENGINE_DEBUG__
        CheckTrafficLimit(netEntityId, static_cast<uint32>(diffSize));
#endif

        DVASSERT(diffSize > 0, "Snapshot diff size bigger than BIG_BUFFER_SIZE");
        if (!diffSize)
        {
            Logger::Error("Snapshot diff size bigger than BIG_BUFFER_SIZE");
            tmpBlock.size = 0;
            return WriteResult::EXCEPTION;
        }

        if (!params.frameIdBase)
        {
            range.baseFrameId = 0;
            range.sharedFrameId = 0;
        }

        EntityHeader entHeader;
        entHeader.netEntityId = netEntityId;
        entHeader.frameOffset = range.GetFrameOffset(params.frameIdBase);
        entHeader.Save(tmpBlock.buff + emptyPacketHeaderSize);
        tmpBlock.size = static_cast<uint32>(tmpHeadersOffset + diffSize);

        if (0 == entHeader.frameOffset || tmpBlock.size > NetworkCoreUtils::ENET_DEFAULT_MTU_UNCOMPRESSED)
        {
            DVASSERT(tmpBlock.size <= NetworkCoreUtils::ENET_DEFAULT_MTU_UNCOMPRESSED || 0 == entHeader.frameOffset,
                     "Size tmpger than MTU is possible only for full sync");
            /*
             *  Huge and full diff are sent by reliable channel.
             *  Sooner or later client will receive full sync and depended diff together.
             */
            range.baseFrameId = range.currFrameId;
            range.sharedFrameId = 0;
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
        SequenceData& seqData = env.seqToSentFrames[env.sequenceId % MaxSentCount];
        if (seqData.frameId != env.frameId)
        {
            if (seqData.frameId > 0)
            {
                // Client hasn't confirmed this seqId.
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
    PacketParams packetParams = PacketParams::Unreliable(PacketParams::DELTA_REPLICATION_CHANNEL_ID);
    env.pktHeader.frameId = env.frameId;
    env.pktHeader.sequenceId = env.sequenceId;
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
    const PacketParams packetParams = PacketParams::Reliable(PacketParams::DELTA_REPLICATION_CHANNEL_ID);
    tmpPacketHeader.Save(tmpBlock.buff);
    env.responder.Send(tmpBlock.buff, tmpBlock.size, packetParams);
    tmpBlock.size = 0;

    ++env.currPktCount;
}

void NetworkDeltaReplicationSystemServer::ProcessResponder(ResponderData& responderData, NetworkPlayerID playerId)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkDeltaReplicationSystemServer::ProcessResponder");

    using namespace NetworkDeltaReplicationSystemDetail;
    const Responder& responder = server->GetResponder(responderData.token);

    const uint32 frameId = timeComp->GetFrameId();

    SequenceId& sequenceId = responderData.maxSeq;
    if (!sequenceId)
    {
        sequenceId = 1;
    }

    ResponderEnvironment env{ responder, frameId, sequenceId, responderData.baseFrames, responderData.sentFrames };
    NetworkStatisticsSingleComponent* statsComp = GetScene()->GetSingleComponent<NetworkStatisticsSingleComponent>();
    // prob. need bool flag inside NetworkStatisticsSingleComponent here
    //if (statsComp)
    {
        uint64 tsKey = NetStatTimestamps::GetKey(frameId - 1, playerId);
        env.pktHeader.timestamps = statsComp->RemoveTimestamps(tsKey);
        if (env.pktHeader.timestamps)
        {
            env.pktHeader.timestamps->server.sendToNet = static_cast<uint32>(SystemTimer::GetMs());
        }
    }

    mtuBlock.size = env.pktHeader.GetSize();

    ProcessEntity(NetworkID::SCENE_ID, M::OwnershipRelation::OWNER, env);

    if (responderData.playerComponent)
    {
        for (auto& info : responderData.playerComponent->periods)
        {
            const Entity* entity = info.target;
            if (entity->GetParent() != GetScene())
            {
                // Child entities should be replicated via root entity
                continue;
            }

            NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
            const NetworkID netEntityId = netReplComp->GetNetworkID();
            if (info.fresh)
            {
                info.fresh = false;
            }
            else if (info.period > 1)
            {
                const uint8 sendPeriod = static_cast<uint8>(pow(2.0, info.period - 1));
                const uint32 smoothFrameId = frameId + static_cast<uint32>(netEntityId);
                if ((smoothFrameId % sendPeriod) && !IsEntitySharedOnFrame(netEntityId, frameId))
                {
                    /* Throttling. */
                    continue;
                }
            }

            M::OwnershipRelation ownership = GetPlayerOwnershipRelation(playerId, netReplComp->GetNetworkPlayerID());
            ProcessEntity(netEntityId, ownership, env);
        }
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

void NetworkDeltaReplicationSystemServer::ProcessEntity(NetworkID netEntityId, M::OwnershipRelation ownership, ResponderEnvironment& env)
{
    switch (WriteEntity(netEntityId, ownership, env))
    {
    case WriteResult::MTU_OVERFLOW:
    {
        SendMtuBlock(env);
        WriteResult result = WriteEntity(netEntityId, ownership, env);
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
    NetworkEntitiesSingleComponent* networkEntities = GetScene()->GetSingleComponent<NetworkEntitiesSingleComponent>();
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
                for (auto& singleComp : GetScene()->singleComponents)
                {
                    size += DumpComponent(ReflectedObject(singleComp.second), stream);
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
