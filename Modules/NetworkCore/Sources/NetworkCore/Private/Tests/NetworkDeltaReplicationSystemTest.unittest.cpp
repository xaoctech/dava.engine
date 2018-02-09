#include "UnitTests/UnitTests.h"
#include "Engine/Engine.h"
#include "Scene3D/Scene.h"
#include "Entity/SceneSystem.h"
#include "Entity/Component.h"
#include "Base/BaseTypes.h"

#if !defined(__DAVAENGINE_ANDROID__)

#include "NetworkCore/UDPTransport/UDPServer.h"
#include "NetworkCore/UDPTransport/UDPClient.h"
#include "NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemClient.h"
#include "NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemServer.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkVisibilitySingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"

using namespace DAVA;

const uint8 EPOCH_NUM = 100;
const uint8 MAX_ATTEMPT = 100;
const float32 TICK = 1.f / 60.f;

class UDPResponderMock : public Responder
{
public:
    void Send(const uint8* data_, size_t size_, const PacketParams& param) const override
    {
        data = data_;
        size = size_;
    };
    uint32 GetRtt() const override
    {
        return 0;
    };
    float32 GetPacketLoss() const override
    {
        return 0.f;
    };
    const FastName& GetToken() const override
    {
        return token;
    };
    void SetToken(const FastName& token_) override{};
    bool IsValid() const override
    {
        return false;
    };
    void SetIsValid(bool value) override{};
    const uint8 GetTeamID() const override
    {
        return 0;
    };
    void SetTeamID(uint8 teamID_) override{};
    ENetPeer* GetPeer() const override
    {
        return nullptr;
    };
    void SaveRtt() override{};
    bool RttIsBetter() const override
    {
        return false;
    };

    mutable const uint8* data = nullptr;
    mutable size_t size = 0;

    NetworkPlayerID ID = 1;

private:
    FastName token;
};

class UDPServerMock : public IServer
{
public:
    bool Update(uint32 timeout = 0) override
    {
        return true;
    };
    void Broadcast(const uint8* data, size_t size, const PacketParams& param) const override{};
    void Foreach(const DoForEach& callback) const override
    {
        callback(responder);
    };
    uint32 GetMaxRtt() const override
    {
        return 0;
    };
    const Responder& GetResponder(const FastName& token) const override
    {
        return responder;
    };

    bool HasResponder(const FastName& token) const override
    {
        return true;
    };

    void SetValidToken(const FastName& token) override{};
    void Disconnect(const FastName& token) override{};

    void SubscribeOnConnect(const OnServerConnectCb& callback) override{};
    void SubscribeOnError(const OnServerErrorCb& callback) override{};
    void SubscribeOnReceive(uint8 channel, const OnServerReceiveCb& callback) override{};
    void SubscribeOnTokenConfirmation(const OnServerTokenConfirmationCb& callback) override{};
    void SubscribeOnDisconnect(const OnServerDisconnectCb& callback) override{};

    UDPResponderMock responder;
};

class UDPClientMock : public IClient
{
public:
    bool Update(uint32 timeout = 0) override
    {
        return true;
    };
    bool IsConnected() const override
    {
        return true;
    };
    bool Send(const uint8* data_, size_t size_, const PacketParams& param) const override
    {
        data = data_;
        size = size_;
        return true;
    };

    void SubscribeOnConnect(const OnClientConnectCb& callback) override{};
    void SubscribeOnDisconnect(const OnClientDisconnectCb& callback) override{};
    void SubscribeOnError(const OnClientErrorCb& callback) override{};
    void SubscribeOnReceive(uint8 channel, const OnClientReceiveCb& callback) override{};

    const FastName& GetAuthToken() const override
    {
        return token;
    };
    uint32 GetPing() const override
    {
        return 0;
    };
    float32 GetPacketLoss() const override
    {
        return 0.f;
    }

    mutable const uint8* data = nullptr;
    mutable size_t size = 0;

private:
    FastName token;
};

struct EntityInfo
{
    uint32 baseFrameId = 0;
    uint32 frameId = 0;
    int64 value = 0;
};

using Info = UnorderedMap<NetworkID, EntityInfo>;

struct DeltaReplicationSystemServerMock : public NetworkDeltaReplicationSystemServer
{
    DeltaReplicationSystemServerMock(Scene* scene)
        : NetworkDeltaReplicationSystemServer(scene)
    {
    }

    size_t CreateDiff(SnapshotSingleComponent::CreateDiffParams& params) override
    {
        EntityInfo& eInfo = info[params.entityId];
        eInfo.frameId = params.frameId;
        eInfo.baseFrameId = params.frameIdBase;
        size_t ret;

        auto diffResultIt = emptyDiffResults.find(NetworkID(params.entityId));
        const bool isEmpty = (diffResultIt == emptyDiffResults.end()) ? false : diffResultIt->second;
        if (!isEmpty)
        {
            ret = NetworkSerialization::Save(params.buff, eInfo.value);
        }
        else
        {
            ret = 0;
        }

        return ret;
    }

    UnorderedMap<NetworkID, bool> emptyDiffResults;
    Vector<uint32> entities = {};
    Info info;
};

struct DeltaReplicationSystemClientMock : public NetworkDeltaReplicationSystemClient
{
    DeltaReplicationSystemClientMock(Scene* scene)
        : NetworkDeltaReplicationSystemClient(scene)
    {
    }

    size_t ApplyDiff(NetworkID entityId, uint32 baseFrameId, uint32 frameId, const uint8* srcBuff, size_t)
    {
        EntityInfo& eInfo = info[entityId];
        eInfo.frameId = frameId;
        eInfo.baseFrameId = baseFrameId;
        auto diffResultIt = applyDiffResults.find(entityId);
        size_t ret = 0;
        if (diffResultIt == applyDiffResults.end() || diffResultIt->second)
        {
            ret = (srcBuff) ? NetworkSerialization::Load(srcBuff, eInfo.value) : 0;
        }
        return ret;
    }

    bool HasBaseFrame(uint32 frameId)
    {
        return true;
    }

    UnorderedMap<NetworkID, bool> applyDiffResults;
    Info info;
};

struct CommonContext
{
    CommonContext()
    {
        scene = new Scene(0);
        netTimeComp = scene->GetSingletonComponent<NetworkTimeSingleComponent>();
    }

    virtual ~CommonContext()
    {
        scene->Release();
    }

    Scene* scene;
    NetworkTimeSingleComponent* netTimeComp;
};

struct ClientContext;
struct ServerContext : public CommonContext
{
    ServerContext()
    {
        auto* netGameModeComp = scene->GetSingletonComponent<NetworkGameModeSingleComponent>();
        scene->GetSingletonComponent<NetworkServerSingleComponent>()->SetServer(&server);

        system = new DeltaReplicationSystemServerMock(scene);
        scene->AddSystem(system);

        netGameModeComp->AddNetworkPlayerID(server.responder.GetToken(), server.responder.ID);
        SetEntitiesCount(1);
    }

    ~ServerContext()
    {
        for (auto it : entities)
        {
            it.second->Release();
        }
    }

    void SetEntitiesCount(uint32 entitiesCount)
    {
        auto* netEntitiesComp = scene->GetSingletonComponent<NetworkEntitiesSingleComponent>();
        system->entities.clear();
        for (uint16 entityId = 1; entityId <= entitiesCount; ++entityId)
        {
            Entity* entity = new Entity();
            entity->SetID(entityId);
            NetworkID networkID(entityId);
            netEntitiesComp->RegisterEntity(networkID, entity);
            NetworkReplicationComponent* netReplComp = new NetworkReplicationComponent();
            netReplComp->SetNetworkID(networkID);
            entity->AddComponent(netReplComp);
            system->entities.push_back(entityId);
            entities[networkID] = entity;
            SetVisibility(networkID, 1);
        }
    }

    void SetVisibility(NetworkID entityId, uint8 frequency)
    {
        auto* netVisComp = scene->GetSingletonComponent<NetworkVisibilitySingleComponent>();
        netVisComp->SetVisibility(server.responder.ID, entities[entityId], frequency);
    }

    DeltaReplicationSystemServerMock* system;
    UDPServerMock server;
    UnorderedMap<NetworkID, Entity*> entities;

    void SendTo(ClientContext& clientCtx);
};

struct ClientContext : public CommonContext
{
    ClientContext()
    {
        scene->GetSingletonComponent<NetworkClientSingleComponent>()->SetClient(&client);
        system = new DeltaReplicationSystemClientMock(scene);
        scene->AddSystem(system);
    }

    DeltaReplicationSystemClientMock* system;
    UDPClientMock client;

    void SendTo(ServerContext& serverCtx)
    {
        serverCtx.system->OnReceiveCallback(serverCtx.server.responder, client.data, client.size);
    }
};

void ServerContext::SendTo(ClientContext& clientCtx)
{
    clientCtx.system->OnReceiveCallback(server.responder.data, server.responder.size, 0, 0);
}

DAVA_TESTCLASS (NetworkDeltaReplicationSystemTest)
{
    DAVA_TEST (SimplePositiveTest)
    {
        ServerContext serverCtx;
        ClientContext clientCtx;

        const auto SimulateDataTransmission = [&]()
        {
            clientCtx.system->OnReceiveCallback(serverCtx.server.responder.data, serverCtx.server.responder.size, 0, 0);
            serverCtx.system->OnReceiveCallback(serverCtx.server.responder, clientCtx.client.data, clientCtx.client.size);
        };

        Info& serverInfo = serverCtx.system->info;
        Info& clientInfo = clientCtx.system->info;
        uint32 replicatedValue = 100;
        const NetworkID networkID(1);
        for (uint32 frameId = 1; frameId <= EPOCH_NUM; ++frameId)
        {
            ++replicatedValue;
            serverCtx.netTimeComp->SetFrameId(frameId);
            serverInfo[networkID].value = replicatedValue;
            serverCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(serverCtx.server.responder.size > 0);
            serverCtx.SendTo(clientCtx);

            clientCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(clientCtx.client.size > 0);
            clientCtx.SendTo(serverCtx);

            TEST_VERIFY(clientInfo[networkID].value == replicatedValue);
            TEST_VERIFY(clientInfo[networkID].frameId == frameId);
            TEST_VERIFY(clientInfo[networkID].baseFrameId == frameId - 1);
        }
    }

    DAVA_TEST (LostHalfPacketsTest)
    {
        ServerContext serverCtx;
        ClientContext clientCtx;
        Info& serverInfo = serverCtx.system->info;
        Info& clientInfo = clientCtx.system->info;
        const NetworkID networkID(1);
        uint32 replicatedValue = 100;
        UnorderedSet<uint32> replicatedValues;
        for (uint32 frameId = 1; frameId <= EPOCH_NUM; ++frameId)
        {
            ++replicatedValue;
            serverCtx.netTimeComp->SetFrameId(frameId);
            serverInfo[networkID].value = replicatedValue;
            serverCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(serverCtx.server.responder.size > 0);
            clientCtx.client.size = 0;
            if (frameId % 2)
            {
                serverCtx.SendTo(clientCtx);
            }

            clientCtx.system->ProcessFixed(TICK);
            clientCtx.SendTo(serverCtx);
            replicatedValues.insert(static_cast<uint32>(clientInfo[networkID].value));
        }

        TEST_VERIFY(replicatedValues.size() == (EPOCH_NUM / 2.0));
    }

    DAVA_TEST (ManyEntitiesTest)
    {
        ServerContext serverCtx;
        ClientContext clientCtx;
        const auto ENTITY_NUM = 10;
        serverCtx.SetEntitiesCount(ENTITY_NUM);

        Info& serverInfo = serverCtx.system->info;
        Info& clientInfo = clientCtx.system->info;

        uint32 replicatedValue = 100;
        for (uint32 frameId = 1; frameId <= EPOCH_NUM; ++frameId)
        {
            ++replicatedValue;
            serverCtx.netTimeComp->SetFrameId(frameId);
            for (uint32 entityId = 1; entityId <= ENTITY_NUM; ++entityId)
            {
                serverInfo[NetworkID(entityId)].value = replicatedValue + entityId;
            }

            serverCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(serverCtx.server.responder.size > 0);
            serverCtx.SendTo(clientCtx);

            clientCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(clientCtx.client.size > 0);
            clientCtx.SendTo(serverCtx);

            for (uint32 entityId = 1; entityId <= ENTITY_NUM; ++entityId)
            {
                NetworkID networkID(entityId);
                TEST_VERIFY(clientInfo[networkID].value == replicatedValue + entityId);
                TEST_VERIFY(clientInfo[networkID].frameId == frameId);
                TEST_VERIFY(clientInfo[networkID].baseFrameId == frameId - 1);
            }
        }
    }

    DAVA_TEST (NackAnyEntityInSequenceTest)
    {
        ServerContext serverCtx;
        ClientContext clientCtx;
        const auto ENTITY_NUM = 10;
        const NetworkID lostEntityId(5);
        serverCtx.SetEntitiesCount(ENTITY_NUM);
        clientCtx.system->applyDiffResults[lostEntityId] = false;

        Info& serverInfo = serverCtx.system->info;
        Info& clientInfo = clientCtx.system->info;

        uint32 replicatedValue = 100;
        for (uint32 frameId = 1; frameId <= EPOCH_NUM; ++frameId)
        {
            if (frameId > EPOCH_NUM - 2)
            {
                clientCtx.system->applyDiffResults[lostEntityId] = true;
            }

            ++replicatedValue;
            serverCtx.netTimeComp->SetFrameId(frameId);
            for (uint32 entityId = 1; entityId <= ENTITY_NUM; ++entityId)
            {
                serverInfo[NetworkID(entityId)].value = replicatedValue + entityId;
            }

            serverCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(serverCtx.server.responder.size > 0);
            serverCtx.SendTo(clientCtx);

            clientCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(clientCtx.client.size > 0);
            clientCtx.SendTo(serverCtx);

            for (uint32 entityId = 1; entityId <= ENTITY_NUM; ++entityId)
            {
                NetworkID networkID(entityId);
                TEST_VERIFY(clientInfo[networkID].value == replicatedValue + entityId);
                TEST_VERIFY(clientInfo[networkID].frameId == frameId);
                if (frameId < EPOCH_NUM && networkID == lostEntityId)
                {
                    TEST_VERIFY(clientInfo[networkID].baseFrameId == 0);
                }
                else
                {
                    TEST_VERIFY(clientInfo[networkID].baseFrameId == frameId - 1);
                }
            }
        }
    }

    DAVA_TEST (ZeroDiffTest)
    {
        ServerContext serverCtx;
        ClientContext clientCtx;
        const NetworkID networkID(1);

        Info& serverInfo = serverCtx.system->info;
        Info& clientInfo = clientCtx.system->info;

        serverCtx.system->emptyDiffResults[networkID] = true;
        for (uint32 frameId = 1; frameId <= EPOCH_NUM; ++frameId)
        {
            serverCtx.netTimeComp->SetFrameId(frameId);

            serverCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(serverCtx.server.responder.size > 0);
            serverCtx.SendTo(clientCtx);

            clientCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(clientCtx.client.size > 0);
            clientCtx.SendTo(serverCtx);

            const EntityInfo& eInfno = clientInfo[networkID];
            TEST_VERIFY(eInfno.frameId == frameId);
            TEST_VERIFY(eInfno.baseFrameId == frameId - 1);
        }
    }

    DAVA_TEST (VisibilityThrottleTest)
    {
        ServerContext serverCtx;
        ClientContext clientCtx;
        const NetworkID networkID(1);

        serverCtx.SetVisibility(networkID, 2);

        Info& serverInfo = serverCtx.system->info;
        Info& clientInfo = clientCtx.system->info;
        uint32 replicatedValue = 100;
        UnorderedSet<uint32> replicatedValues;
        for (uint32 frameId = 1; frameId <= EPOCH_NUM; ++frameId)
        {
            serverCtx.netTimeComp->SetFrameId(frameId);
            ++replicatedValue;
            serverInfo[networkID].value = replicatedValue;
            serverCtx.system->ProcessFixed(TICK);
            serverCtx.SendTo(clientCtx);
            clientCtx.system->ProcessFixed(TICK);
            clientCtx.SendTo(serverCtx);
            replicatedValues.insert(static_cast<uint32>(clientInfo[networkID].value));
        }

        TEST_VERIFY(replicatedValues.size() == (EPOCH_NUM / 2.0));
    }

    DAVA_TEST (VisibilityRealTimeChangeTest)
    {
        ServerContext serverCtx;
        ClientContext clientCtx;
        const NetworkID networkID(1);

        serverCtx.SetVisibility(networkID, 1);

        Info& serverInfo = serverCtx.system->info;
        Info& clientInfo = clientCtx.system->info;
        uint32 replicatedValue = 100;
        UnorderedSet<uint32> replicatedValues;
        for (uint32 frameId = 1; frameId <= EPOCH_NUM; ++frameId)
        {
            const bool isViz = (frameId % 2);
            ++replicatedValue;
            serverCtx.SetVisibility(networkID, isViz);
            serverCtx.netTimeComp->SetFrameId(frameId);
            serverInfo[networkID].value = replicatedValue;
            serverCtx.system->ProcessFixed(TICK);
            serverCtx.SendTo(clientCtx);
            clientCtx.system->ProcessFixed(TICK);
            clientCtx.SendTo(serverCtx);
            replicatedValues.insert(static_cast<uint32>(clientInfo[networkID].value));
        }

        TEST_VERIFY(replicatedValues.size() == (EPOCH_NUM / 2.0));
    }

    DAVA_TEST (RemoveEntityPositiveTest)
    {
        ServerContext serverCtx;
        ClientContext clientCtx;
        uint8 ENTITY_NUM = 10;
        const uint32 FRAME_FOR_REMOVE = 10;
        const uint32 REMOVE_ENT_ID = 10;
        serverCtx.SetEntitiesCount(ENTITY_NUM);

        Info& serverInfo = serverCtx.system->info;
        Info& clientInfo = clientCtx.system->info;

        for (uint32 frameId = 1; frameId <= EPOCH_NUM; ++frameId)
        {
            clientInfo.clear();
            if (FRAME_FOR_REMOVE == frameId)
            {
                serverCtx.system->RemoveEntity(serverCtx.entities[NetworkID(REMOVE_ENT_ID)]);
                serverCtx.SetEntitiesCount(--ENTITY_NUM);
            }

            serverCtx.netTimeComp->SetFrameId(frameId);
            for (uint32 entityId = 1; entityId <= ENTITY_NUM; ++entityId)
            {
                NetworkID networkID(REMOVE_ENT_ID);
                if (REMOVE_ENT_ID == entityId && frameId >= FRAME_FOR_REMOVE)
                {
                    serverInfo[networkID].value = -static_cast<int32>(frameId);
                }
                else
                {
                    serverInfo[networkID].value = frameId;
                }
            }

            serverCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(serverCtx.server.responder.size > 0);
            serverCtx.SendTo(clientCtx);

            clientCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(clientCtx.client.size > 0);
            clientCtx.SendTo(serverCtx);

            for (uint32 entityId = 1; entityId <= ENTITY_NUM; ++entityId)
            {
                NetworkID networkID(entityId);
                if (REMOVE_ENT_ID == entityId && frameId >= FRAME_FOR_REMOVE)
                {
                    if (FRAME_FOR_REMOVE == frameId)
                    {
                        TEST_VERIFY(clientInfo[networkID].value == -static_cast<int32>(FRAME_FOR_REMOVE));
                    }
                    else
                    {
                        TEST_VERIFY(clientInfo.find(networkID) == clientInfo.end());
                    }
                }
                else
                {
                    TEST_VERIFY(clientInfo[networkID].value == frameId);
                }
            }
        }
    }

    DAVA_TEST (RemoveEntityClientDisconnectTest)
    {
        ServerContext serverCtx;
        ClientContext clientCtx;
        uint8 ENTITY_NUM = 10;
        const uint32 FRAME_FOR_REMOVE = 10;
        const uint32 REMOVE_ENT_ID = 10;
        serverCtx.SetEntitiesCount(ENTITY_NUM);

        Info& serverInfo = serverCtx.system->info;
        Info& clientInfo = clientCtx.system->info;

        for (uint32 frameId = 1; frameId <= EPOCH_NUM; ++frameId)
        {
            const bool isReconnected = (frameId > EPOCH_NUM - 2);
            clientInfo.clear();
            if (FRAME_FOR_REMOVE == frameId)
            {
                serverCtx.system->RemoveEntity(serverCtx.entities[NetworkID(REMOVE_ENT_ID)]);
                serverCtx.SetEntitiesCount(--ENTITY_NUM);
            }

            serverCtx.netTimeComp->SetFrameId(frameId);
            for (uint32 entityId = 1; entityId <= ENTITY_NUM; ++entityId)
            {
                NetworkID networkID(entityId);
                if (REMOVE_ENT_ID == entityId && frameId >= FRAME_FOR_REMOVE)
                {
                    serverInfo[networkID].value = -static_cast<int32>(frameId);
                }
                else
                {
                    serverInfo[networkID].value = frameId;
                }
            }

            serverCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(serverCtx.server.responder.size > 0);
            serverCtx.SendTo(clientCtx);

            clientCtx.system->ProcessFixed(TICK);
            TEST_VERIFY(clientCtx.client.size > 0);
            if (frameId < FRAME_FOR_REMOVE || isReconnected)
            {
                clientCtx.SendTo(serverCtx);
            }

            if (!isReconnected)
            {
                for (uint32 entityId = 1; entityId <= ENTITY_NUM; ++entityId)
                {
                    NetworkID networkID(entityId);
                    if (REMOVE_ENT_ID == entityId && frameId >= FRAME_FOR_REMOVE)
                    {
                        TEST_VERIFY(clientInfo[networkID].value == -static_cast<int32>(frameId));
                    }
                    else
                    {
                        TEST_VERIFY(clientInfo[networkID].value == frameId);
                    }
                }
            }
        }

        TEST_VERIFY(clientInfo.find(NetworkID(REMOVE_ENT_ID)) == clientInfo.end())
    }
};

DAVA_TESTCLASS (ElasticBufferTest)
{
    DAVA_TEST (AllocateTest)
    {
        ElasticBuffer buffer;
        const size_t srcSize = ElasticBuffer::PRIMARY_BUFF_SIZE / 1024;
        const std::unique_ptr<uint8[]> srcBuff(new uint8[srcSize]);

        size_t offset = 0;
        for (; offset < ElasticBuffer::PRIMARY_BUFF_SIZE - srcSize;)
        {
            offset += srcSize;
            const uint8* res = buffer.Insert(srcBuff.get(), srcSize);
            TEST_VERIFY(nullptr != res);
            TEST_VERIFY(buffer.GetOffset() == offset);
            TEST_VERIFY(buffer.GetFallbackCount() == 0);
            TEST_VERIFY(&buffer.GetTail() == &buffer);
        }

        const size_t fallbackSize = ElasticBuffer::FALLBACK_BUFF_SIZE;
        const std::unique_ptr<uint8[]> fallbackBuff(new uint8[fallbackSize]);

        const size_t epochCount = 3;
        for (size_t epochId = 1; epochId <= epochCount; ++epochId)
        {
            const uint8* res = buffer.Insert(fallbackBuff.get(), fallbackSize);
            TEST_VERIFY(nullptr != res);
            TEST_VERIFY(buffer.GetOffset() == offset);
            TEST_VERIFY(buffer.GetFallbackCount() == epochId);
            TEST_VERIFY(&buffer.GetTail() != &buffer);
            TEST_VERIFY(buffer.GetTail().GetOffset() == fallbackSize);
        }

        buffer.Reset();
        TEST_VERIFY(buffer.GetOffset() == 0);
        TEST_VERIFY(buffer.GetFallbackCount() == 0);
        TEST_VERIFY(&buffer.GetTail() == &buffer);
    }
};

#endif // !defined(__DAVAENGINE_ANDROID__)
