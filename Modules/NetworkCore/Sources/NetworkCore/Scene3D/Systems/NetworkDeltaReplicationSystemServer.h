#pragma once

#include <Entity/SceneSystem.h>
#include <Base/BaseTypes.h>


#include "NetworkCore/Private/NetworkSerialization.h"
#include "NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemBase.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
class NetworkVisibilitySingleComponent;
class NetworkGameModeSingleComponent;
class NetworkTimeSingleComponent;

class NetworkDeltaReplicationSystemServer : public NetworkDeltaReplicationSystemBase
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkDeltaReplicationSystemServer, NetworkDeltaReplicationSystemBase);

    NetworkDeltaReplicationSystemServer(Scene* scene);

    void RemoveEntity(Entity* entity) override;
    void ProcessFixed(float32 timeElapsed) override;
    void OnReceiveCallback(const Responder& responder, const uint8* data, size_t size);
    void OnClientDisconnect(const FastName& token);

protected:
    virtual size_t CreateDiff(SnapshotSingleComponent::CreateDiffParams& params);

private:
    const PacketHeader emptyPacketHeader = {};
    const uint32 emptyPacketHeaderSize = emptyPacketHeader.GetSize();
    const EntityHeader emptyEntityHeader = {};
    const uint32 entHeaderSize = emptyEntityHeader.GetSize();
    static const SequenceId MAX_SENT_COUNT = 1000;

    PreAllocatedBlock tmpBlock = {};

    struct FrameRange
    {
        uint8 GetFrameCount() const
        {
            DVASSERT(baseFrameId <= currFrameId);
            if (baseFrameId > 0)
            {
                uint32 diff = currFrameId - baseFrameId;
                return static_cast<uint8>((diff < uint8(~0)) ? diff : 0);
            }

            return 0;
        }

        void Clear()
        {
            baseFrameId = 0;
            currFrameId = 0;
        }

        uint32 baseFrameId = 0;
        uint32 currFrameId = 0;
        uint32 delFrameId = 0;
    };

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

    using SeqToSentFrames = Array<SequenceData, MAX_SENT_COUNT>;
    using EntityToBaseFrames = UnorderedMap<NetworkID, FrameRange>;
    using RemovedEntityToPrivacy = UnorderedMap<NetworkID, M::Privacy>;

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
        SequenceId maxSeq;
        RemovedEntityToPrivacy removedEntities;
    };

    void ProcessAckPackets(const Responder& responder, ResponderData& responderData);
    void ProcessResponder(const Responder& responder, ResponderData& responderData);
    void ProcessEntity(NetworkID netEntityId, M::Privacy privacy, ResponderEnvironment& env);
    WriteResult WriteEntity(NetworkID netEntityId, M::Privacy privacy, ResponderEnvironment& env);
    void SendMtuBlock(ResponderEnvironment& env);
    void SendTmpBlock(ResponderEnvironment& env);

    M::Privacy GetPrivacy(NetworkPlayerID playerId, NetworkPlayerID entityPlayerId);

    UnorderedMap<const Responder*, ResponderData> respondersData;

    void CheckTrafficLimit(NetworkID netEntityId, uint32 diffSize);
    uint32 DumpComponent(const ReflectedObject& reflectedObject, StringStream& stream);

    IServer* server;

    SnapshotSingleComponent* snapshotSingleComponent;
    NetworkVisibilitySingleComponent* netVisSingleComp;
    NetworkGameModeSingleComponent* netGameModeComp;
    NetworkTimeSingleComponent* timeComp;
};

} //namespace DAVA
