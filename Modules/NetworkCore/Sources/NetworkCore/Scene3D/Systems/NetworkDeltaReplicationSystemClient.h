#pragma once

#include <Entity/SceneSystem.h>
#include <Base/BaseTypes.h>

#include "NetworkCore/Private/NetworkSerialization.h"
#include "NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemBase.h"

namespace DAVA
{
class IClient;
class NetworkReplicationSingleComponent;
class NetworkDeltaSingleComponent;
class NetworkStatisticsSingleComponent;
class NetworkClientConnectionSingleComponent;

class NetworkDeltaReplicationSystemClient : public NetworkDeltaReplicationSystemBase
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkDeltaReplicationSystemClient, NetworkDeltaReplicationSystemBase);

    NetworkDeltaReplicationSystemClient(Scene* scene);
    void ProcessReceivePackets(float32 timeElapsed);
    void ProcessAppliedPackets(float32 timeElapsed);
    void OnReceive(const Vector<uint8>& packet);

private:
    IClient* client;

    NetworkReplicationSingleComponent* replicationSingleComponent;
    NetworkDeltaSingleComponent* deltaSingleComponent;
    NetworkStatisticsSingleComponent* statsComp;
    const NetworkClientConnectionSingleComponent* netConnectionComp;

    ElasticBuffer elasticBuffer;
    UnorderedMap<SequenceId, uint32> sequenceToCounter;

    struct UnreliableFragments
    {
        uint8 all = 0;
        uint8 rcv = 0;
        bool isEntire = false;
    };

    UnorderedMap<uint32, UnreliableFragments> frameToFragments;
};

} //namespace DAVA
