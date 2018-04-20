#pragma once

#include <Entity/SceneSystem.h>
#include <Base/BaseTypes.h>


#include "NetworkCore/Private/NetworkSerialization.h"
#include "NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemBase.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

#include "Scene3D/ComponentGroup.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
class NetworkGameModeSingleComponent;
class NetworkTimeSingleComponent;
class NetworkServerConnectionsSingleComponent;

namespace NetworkDeltaReplicationSystemDetail
{
struct FrameRange;
class DiffCache;
}

class NetworkDeltaReplicationSystemServer : public NetworkDeltaReplicationSystemBase
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkDeltaReplicationSystemServer, NetworkDeltaReplicationSystemBase);

    struct DiffParams
    {
        NetworkID entityId;
        uint32 frameIdBase = 0;
        uint32 frameId = 0;
        M::OwnershipRelation ownership;
        uint8* buff = nullptr;
        size_t buffSize = 0;

        size_t outDiffSize = 0;
    };

    explicit NetworkDeltaReplicationSystemServer(Scene* scene);
    ~NetworkDeltaReplicationSystemServer() override;

    void RemoveEntity(Entity* entity) override;
    void ProcessFixed(float32 timeElapsed) override;
    void OnReceive(const FastName& token, const Vector<uint8>& data);

protected:
    virtual size_t CreateDiff(DiffParams& params);

private:
    const PacketHeader emptyPacketHeader = {};
    const uint32 emptyPacketHeaderSize = emptyPacketHeader.GetSize();
    const EntityHeader emptyEntityHeader = {};
    const uint32 entHeaderSize = emptyEntityHeader.GetSize();

    PreAllocatedBlock tmpBlock = {};
    enum class WriteResult : uint8
    {
        CONTINUE,
        MTU_OVERFLOW,
        FULL_SYNC,
        EXCEPTION,
    };

    struct SequenceData
    {
        Vector<NetworkID> networkIds;
        uint32 frameId = 0;
    };

    using SeqToSentFrames = Array<SequenceData, MaxSentCount>;
    using EntityToBaseFrames = UnorderedMap<NetworkID, NetworkDeltaReplicationSystemDetail::FrameRange>;
    using RemovedEntityToOwnership = UnorderedMap<NetworkID, M::OwnershipRelation>;

    struct ResponderEnvironment
    {
        const Responder& responder;
        uint32 frameId;
        SequenceId& sequenceId;
        EntityToBaseFrames& entityToBaseFrames;
        SeqToSentFrames& seqToSentFrames;

        PacketHeader pktHeader;
        uint8 currPktCount = 0;
    };

    struct ResponderData
    {
        Vector<SequenceId> acks;
        EntityToBaseFrames baseFrames;
        SeqToSentFrames sentFrames;
        RemovedEntityToOwnership removedEntities;
        FastName token;
        NetworkPlayerComponent* playerComponent = nullptr;
        SequenceId maxSeq = 0;
    };

    std::unique_ptr<NetworkDeltaReplicationSystemDetail::DiffCache> diffCache;

    void OnClientConnect(const FastName& token);
    void OnClientDisconnect(const FastName& token);

    void OnPlayerComponentAdded(NetworkPlayerComponent* component);
    void OnPlayerComponentRemoved(NetworkPlayerComponent* component);
    void ProcessAckPackets(ResponderData& responderData);
    void ProcessResponder(ResponderData& responderData, NetworkPlayerID playerId);
    void ProcessEntity(NetworkID netEntityId, M::OwnershipRelation ownership, ResponderEnvironment& env);
    WriteResult WriteEntity(NetworkID netEntityId, M::OwnershipRelation ownership, ResponderEnvironment& env);
    void SendMtuBlock(ResponderEnvironment& env);
    void SendTmpBlock(ResponderEnvironment& env);

    Vector<ResponderData> responderDataList;
    NetworkPlayerID playerIdUpperBound;

    void CheckTrafficLimit(NetworkID netEntityId, uint32 diffSize);
    uint32 DumpComponent(const ReflectedObject& reflectedObject, StringStream& stream);

    IServer* server;

    SnapshotSingleComponent* snapshotSingleComponent;
    const NetworkGameModeSingleComponent* netGameModeComp;
    const NetworkTimeSingleComponent* timeComp;
    ComponentGroup<NetworkPlayerComponent>* playerComponentGroup;
    const NetworkServerConnectionsSingleComponent* netConnectionsComp;
};

} //namespace DAVA
