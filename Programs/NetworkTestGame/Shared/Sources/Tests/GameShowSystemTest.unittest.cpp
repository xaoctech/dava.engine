#include "UnitTests/UnitTests.h"

#include "Engine/Engine.h"
#include "Scene3D/Scene.h"
#include "Entity/SceneSystem.h"
#include "Entity/Component.h"
#include "Base/BaseTypes.h"

#include "Systems/GameShowSystem.h"
#include "NetworkCore/UDPTransport/UDPClient.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h"

#include <Entity/ComponentManager.h>

using namespace DAVA;

DAVA_TESTCLASS (GameShowSystemTest)
{
    DAVA_TEST (ThePlayerIsWatchingAnotherPlayerTest)
    {
        const uint8 ENTITIES_NUM = 3;

        ScopedPtr<Scene> scene(new Scene(0));
        NetworkEntitiesSingleComponent* entitiesSingleComponent = scene->GetSingletonComponent<NetworkEntitiesSingleComponent>();

        struct ClientMock : public IClient
        {
            ClientMock() = default;

            bool Update(uint32 timeout = 0) override
            {
                return std::rand() % 2;
            }
            bool IsConnected() const override
            {
                return std::rand() % 2;
            }
            bool Send(const uint8* data, size_t size, const PacketParams& param) const override
            {
                return std::rand() % 2;
            }

            void SubscribeOnConnect(const OnClientConnectCb& callback) override
            {
            }
            void SubscribeOnDisconnect(const OnClientDisconnectCb& callback) override
            {
            }
            void SubscribeOnError(const OnClientErrorCb& callback) override
            {
            }
            void SubscribeOnReceive(uint8 channel, const OnClientReceiveCb& callback) override
            {
            }

            const FastName& GetAuthToken() const override
            {
                static FastName fn;
                return fn;
            }
            /* on error return std::numeric_limits<uint32>::max() */
            uint32 GetPing() const override
            {
                return {};
            }
            float32 GetPacketLoss() const override
            {
                return {};
            }
        };

        ClientMock client;
        scene->GetSingletonComponent<NetworkClientSingleComponent>()->SetClient(&client);

        GameShowSystem* gameShowSystem = new GameShowSystem(scene);
        scene->AddSystem(gameShowSystem);

        ScopedPtr<Entity> player1(new Entity());
        ScopedPtr<Entity> player2(new Entity());
        const Vector<Entity*> entities = { nullptr, player1, player2 };
        TEST_VERIFY(ENTITIES_NUM == entities.size());
        NetworkPlayerComponent* playerComponent = new NetworkPlayerComponent();
        player1->AddComponent(playerComponent);
        for (uint8 playerID = 1; playerID < ENTITIES_NUM; ++playerID)
        {
            NetworkID networkID(playerID);
            Entity* entity = entities[playerID];
            entitiesSingleComponent->RegisterEntity(networkID, entity);
            TEST_VERIFY(entitiesSingleComponent->FindByID(networkID));
            NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent();
            replicationComponent->SetNetworkPlayerID(playerID);
            replicationComponent->SetNetworkID(networkID);
            replicationComponent->SetOwnerTeamID(playerID % 2);
            entity->AddComponent(replicationComponent);
            scene->AddNode(entity);
        }

        playerComponent->visibleEntityIds.resize(1);
        playerComponent->visibleEntityIds[0] = NetworkID(2);

        gameShowSystem->Process(0);
        TEST_VERIFY(player2->GetVisible());

        playerComponent->visibleEntityIds.resize(0);
        gameShowSystem->Process(0);
        TEST_VERIFY(!player2->GetVisible());

        playerComponent->visibleEntityIds.resize(1);
        playerComponent->visibleEntityIds[0] = NetworkID(2);
        gameShowSystem->Process(0);
        TEST_VERIFY(player2->GetVisible());
    }
};
