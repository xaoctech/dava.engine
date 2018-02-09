#include "UnitTests/UnitTests.h"

#include <Base/BaseTypes.h>
#include <Base/Vector.h>
#include <Base/FastName.h>
#include <Base/RefPtr.h>
#include <Math/Vector.h>
#include <FileSystem/KeyedArchive.h>
#include <Input/InputSystemTypes.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/UDPTransport/UDPClient.h>
#include <NetworkCore/UDPTransport/UDPServer.h>
#include <NetworkCore/Scene3D/Systems/NetworkRemoteInputSystem.h>
#include <NetworkCore/Scene3D/Components/NetworkRemoteInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>

#include <algorithm>

DAVA_TESTCLASS (NetworkRemoteInputSystemTest)
{
    static const DAVA::FastName DIGITAL_ACTION_1;
    static const DAVA::FastName DIGITAL_ACTION_2;
    static const DAVA::FastName ANALOG_ACTION_1;
    static const DAVA::FastName ANALOG_ACTION_2;

    // Mocks common part of an application which runs either server or client logic
    class BaseContext
    {
    public:
        BaseContext()
        {
            using namespace DAVA;

            scene = new Scene(0);

            actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();

            actionsSingleComponent->AddAvailableDigitalAction(DIGITAL_ACTION_1);
            actionsSingleComponent->AddAvailableDigitalAction(DIGITAL_ACTION_2);
            actionsSingleComponent->AddAvailableAnalogAction(ANALOG_ACTION_1, DAVA::AnalogPrecision::ANALOG_UINT16);
            actionsSingleComponent->AddAvailableAnalogAction(ANALOG_ACTION_2, DAVA::AnalogPrecision::ANALOG_UINT16);

            // Start with UINT32_MAX so that next process starts with zero frame by incrementing this value
            networkTimeSingleComponent = scene->GetSingletonComponent<NetworkTimeSingleComponent>();
            networkTimeSingleComponent->SetFrameId(UINT32_MAX);
            networkTimeSingleComponent->SetIsInitialized(true);

            networkReplicationSingleComponent = scene->GetSingletonComponent<NetworkReplicationSingleComponent>();

            remoteInputSystem = new DAVA::NetworkRemoteInputSystem(scene);
            scene->AddSystem(remoteInputSystem);

            entity = new DAVA::Entity();

            remoteInputComponent = new DAVA::NetworkRemoteInputComponent();
            remoteInputComponent->AddActionToReplicate(DIGITAL_ACTION_1);
            remoteInputComponent->AddActionToReplicate(DIGITAL_ACTION_2);
            remoteInputComponent->AddActionToReplicate(ANALOG_ACTION_1);
            remoteInputComponent->AddActionToReplicate(ANALOG_ACTION_2);
            entity->AddComponent(remoteInputComponent);

            replicationComponent = new NetworkReplicationComponent();
            replicationComponent->SetNetworkID(NetworkID(1));
            entity->AddComponent(replicationComponent);

            scene->AddNode(entity);
        }

        virtual ~BaseContext()
        {
            scene->Release();
        }

        virtual void ProcessFrame()
        {
            networkTimeSingleComponent->SetFrameId(networkTimeSingleComponent->GetFrameId() + 1);
        }

        virtual void EndFrame()
        {
            actionsSingleComponent->Clear();
        }

        BaseContext(const BaseContext&) = delete;

        BaseContext& operator=(const BaseContext&) = delete;

        DAVA::Scene* scene;
        DAVA::ActionsSingleComponent* actionsSingleComponent;
        DAVA::NetworkTimeSingleComponent* networkTimeSingleComponent;
        DAVA::NetworkReplicationSingleComponent* networkReplicationSingleComponent;
        DAVA::NetworkRemoteInputSystem* remoteInputSystem;

        DAVA::Entity* entity;
        DAVA::NetworkRemoteInputComponent* remoteInputComponent;
        DAVA::NetworkReplicationComponent* replicationComponent;
    };

    struct ClientMock : public DAVA::IClient
    {
        ClientMock() = default;

        bool Update(DAVA::uint32 timeout = 0) override
        {
            return std::rand() % 2;
        }
        bool IsConnected() const override
        {
            return std::rand() % 2;
        }
        bool Send(const DAVA::uint8* data, size_t size, const DAVA::PacketParams& param) const override
        {
            return std::rand() % 2;
        }

        void SubscribeOnConnect(const DAVA::OnClientConnectCb& callback) override
        {
        }
        void SubscribeOnDisconnect(const DAVA::OnClientDisconnectCb& callback) override
        {
        }
        void SubscribeOnError(const DAVA::OnClientErrorCb& callback) override
        {
        }
        void SubscribeOnReceive(DAVA::uint8 channel, const DAVA::OnClientReceiveCb& callback) override
        {
        }

        const DAVA::FastName& GetAuthToken() const override
        {
            static DAVA::FastName fn;
            return fn;
        }
        /* on error return std::numeric_limits<uint32>::max() */
        DAVA::uint32 GetPing() const override
        {
            return {};
        }
        DAVA::float32 GetPacketLoss() const override
        {
            return {};
        }
    };

    struct ServerMock : public DAVA::IServer
    {
        ServerMock() = default;

        bool Update(DAVA::uint32 timeout = 0) override
        {
            return std::rand() % 2;
        }
        void Broadcast(const DAVA::uint8* data, size_t size, const DAVA::PacketParams& param) const override
        {
        }
        void Foreach(const DAVA::DoForEach& callback) const override
        {
        }
        DAVA::uint32 GetMaxRtt() const override
        {
            return {};
        }
        const DAVA::Responder& GetResponder(const DAVA::FastName& token) const override
        {
            static DAVA::UDPResponder dummy(nullptr, nullptr);
            return dummy;
        }
        bool HasResponder(const DAVA::FastName& token) const override
        {
            return std::rand() % 2;
        }
        void SetValidToken(const DAVA::FastName& token) override
        {
        }
        void Disconnect(const DAVA::FastName& token) override
        {
        }

        void SubscribeOnConnect(const DAVA::OnServerConnectCb& callback) override
        {
        }
        void SubscribeOnError(const DAVA::OnServerErrorCb& callback) override
        {
        }
        void SubscribeOnReceive(DAVA::uint8 channel, const DAVA::OnServerReceiveCb& callback) override
        {
        }
        void SubscribeOnTokenConfirmation(const DAVA::OnServerTokenConfirmationCb& callback) override
        {
        }
        void SubscribeOnDisconnect(const DAVA::OnServerDisconnectCb& callback) override
        {
        }
    };

    class ServerContext : public BaseContext
    {
    public:
        ServerContext()
            : BaseContext()
        {
            using namespace DAVA;

            ServerMock server;
            NetworkServerSingleComponent* serverSingleComponent = scene->GetSingletonComponent<NetworkServerSingleComponent>();
            serverSingleComponent->SetServer(&server);
        }

        void ProcessFrame() override
        {
            BaseContext::ProcessFrame();
            remoteInputSystem->ProcessServer(1.0f / 60.0f);
        }
    };

    class ClientContext : public BaseContext
    {
    public:
        ClientContext()
            : BaseContext()
        {
            using namespace DAVA;

            ClientMock client;
            NetworkClientSingleComponent* serverSingleComponent = scene->GetSingletonComponent<NetworkClientSingleComponent>();
            serverSingleComponent->SetClient(&client);

            NetworkGameModeSingleComponent* gameModeSingleComponent = scene->GetSingletonComponent<NetworkGameModeSingleComponent>();
            gameModeSingleComponent->SetNetworkPlayerID(replicationComponent->GetNetworkPlayerID());
        }

        void ProcessFrame() override
        {
            BaseContext::ProcessFrame();
            remoteInputSystem->ProcessClient(1.0f / 60.0f);
        }
    };

    // Copy RemoteInputComponent from server to client and remember server frame on a client
    void ReplicateRemoteInputComponent(ServerContext & server, ClientContext & client)
    {
        using namespace DAVA;

        RefPtr<KeyedArchive> archive(new KeyedArchive);
        server.remoteInputComponent->Serialize(archive.Get(), nullptr);
        client.remoteInputComponent->Deserialize(archive.Get(), nullptr);

        NetworkID networkID = client.entity->GetComponent<NetworkReplicationComponent>()->GetNetworkID();
        client.networkReplicationSingleComponent->replicationInfo[networkID].frameIdServer = client.networkTimeSingleComponent->GetFrameId() + 1;
    }

    // Roll forward on a server or a client so that they both are on the same frame
    void SyncServerClient(ServerContext & server, ClientContext & client)
    {
        using namespace DAVA;

        uint32 serverFrame = server.networkTimeSingleComponent->GetFrameId();
        uint32 clientFrame = client.networkTimeSingleComponent->GetFrameId();
        if (serverFrame > clientFrame)
        {
            for (uint32 i = 0; i < serverFrame - clientFrame; ++i)
            {
                client.ProcessFrame();
                client.EndFrame();
            }
        }
        else if (clientFrame > serverFrame)
        {
            for (uint32 i = 0; i < clientFrame - serverFrame; ++i)
            {
                server.ProcessFrame();
                server.EndFrame();
            }
        }
    }

    // Return `true` if this vector contains no actual input
    bool AreInputsEmpty(const DAVA::Vector<DAVA::ActionsSingleComponent::Actions>& allActions)
    {
        if (allActions.size() == 0)
        {
            return true;
        }

        for (const DAVA::ActionsSingleComponent::Actions& actions : allActions)
        {
            if (actions.digitalActions.size() > 0)
            {
                return false;
            }

            if (actions.analogActions.size() > 0)
            {
                return false;
            }
        }

        return true;
    }

    struct Input
    {
        DAVA::Vector<DAVA::FastName> digitalActions;
        DAVA::Vector<DAVA::FastName> analogActions;
        DAVA::Vector<DAVA::Vector2> analogActionsValues;
    };

    // Apply specified input for all passed frames on server (`serverInput` = vector of inputs for frames)
    // Copy RemoteInputComponent to client and process frames (including `numPredictions` additional frames)
    // Return all client input that was resulted from these actions (i.e. replicated and/or predicted by client)
    DAVA::Vector<DAVA::ActionsSingleComponent::Actions> ReplicateServerInput(ServerContext & server, ClientContext & client, const DAVA::Vector<Input>& serverInput, DAVA::uint32 numPredictions, bool sync = true)
    {
        using namespace DAVA;

        // Should not be an owner for input to be replicated
        TEST_VERIFY(!IsClientOwner(client.scene, client.entity));

        if (sync)
        {
            SyncServerClient(server, client);
        }

        // Apply input on server for all specified frames
        for (uint32 i = 0; i < serverInput.size(); ++i)
        {
            for (const FastName& action : serverInput[i].digitalActions)
            {
                AddDigitalActionForClient(server.scene, server.entity, action);
            }

            for (size_t j = 0; j < serverInput[i].analogActions.size(); ++j)
            {
                AddAnalogActionForClient(server.scene, server.entity, serverInput[i].analogActions[j], serverInput[i].analogActionsValues[j]);
            }

            server.ProcessFrame();
            server.EndFrame();
        }

        // Roll forward until we're one frame before the one on which this input was applied on server
        // To avoid unneeded predictions (they will be tested later on)
        int64 diff = static_cast<int64>(server.networkTimeSingleComponent->GetFrameId()) - static_cast<int64>(client.networkTimeSingleComponent->GetFrameId());
        if (diff > 0)
        {
            for (uint32 i = 0; i < static_cast<uint32>(diff) - 1; ++i)
            {
                client.ProcessFrame();
                client.EndFrame();
            }
        }

        ReplicateRemoteInputComponent(server, client);

        // Client should restore and apply input
        client.ProcessFrame();

        // Save result
        Vector<ActionsSingleComponent::Actions> result;

        Vector<ActionsSingleComponent::Actions> nonPredictedInput = client.actionsSingleComponent->GetActions(client.replicationComponent->GetNetworkPlayerID());
        result.insert(result.end(), nonPredictedInput.begin(), nonPredictedInput.end());

        // Finish client frame
        client.EndFrame();

        for (uint32 i = 0; i < numPredictions; ++i)
        {
            client.ProcessFrame();

            Vector<ActionsSingleComponent::Actions> predictedInput = client.actionsSingleComponent->GetActions(client.replicationComponent->GetNetworkPlayerID());
            if (!AreInputsEmpty(predictedInput))
            {
                result.insert(result.end(), predictedInput.begin(), predictedInput.end());
            }

            client.EndFrame();
        }

        return std::move(result);
    }

    void VerifyAnalogAction(DAVA::ActionsSingleComponent::Actions & actions, DAVA::FastName actionId, DAVA::Vector2 value)
    {
        using namespace DAVA;

        TEST_VERIFY(actions.analogActions.find({ actionId, AnalogPrecision::ANALOG_UINT16 }) != actions.analogActions.end());

        Vector2 packedValue = actions.analogActions[{ actionId, AnalogPrecision::ANALOG_UINT16 }];
        Vector2 unpackedValue = ConvertFixedPrecisionToAnalog(AnalogPrecision::ANALOG_UINT16, packedValue);

        static const float32 EPSILON = 0.01f;
        TEST_VERIFY(FLOAT_EQUAL_EPS(unpackedValue.x, value.x, EPSILON));
        TEST_VERIFY(FLOAT_EQUAL_EPS(unpackedValue.y, value.y, EPSILON));
    }

    DAVA_TEST (TestReplication)
    {
        using namespace DAVA;

        ServerContext server;
        ClientContext client;

        // Set network player id to 1 so that they represent entity which is not owned by a client
        server.replicationComponent->SetNetworkPlayerID(1);
        client.replicationComponent->SetNetworkPlayerID(1);

        // One frame - one action, no prediction
        {
            Input input;
            input.digitalActions.push_back(DIGITAL_ACTION_1);

            Vector<ActionsSingleComponent::Actions> clientInput = ReplicateServerInput(server, client, { input }, 0);
            TEST_VERIFY(clientInput.size() == 1);
            TEST_VERIFY(clientInput[0].digitalActions.size() == 1);
            TEST_VERIFY(clientInput[0].digitalActions[0] == DIGITAL_ACTION_1);
            TEST_VERIFY(clientInput[0].analogActions.size() == 0);
        }

        // One frame - multiple duplicated inputs, no prediction
        {
            Input input;
            input.digitalActions.push_back(DIGITAL_ACTION_1);
            input.digitalActions.push_back(DIGITAL_ACTION_1);
            input.digitalActions.push_back(DIGITAL_ACTION_1);
            input.digitalActions.push_back(DIGITAL_ACTION_1);

            Vector<ActionsSingleComponent::Actions> clientInput = ReplicateServerInput(server, client, { input }, 0);
            TEST_VERIFY(clientInput.size() == 1);
            TEST_VERIFY(clientInput[0].digitalActions.size() == 1);
            TEST_VERIFY(clientInput[0].digitalActions[0] == DIGITAL_ACTION_1);
            TEST_VERIFY(clientInput[0].analogActions.size() == 0);
        }

        // Two frames - multiple actions per frame, no duplicates, one predicted frame
        // Then check correction for single frame
        {
            Input input0;
            input0.digitalActions.push_back(DIGITAL_ACTION_1);
            input0.digitalActions.push_back(DIGITAL_ACTION_2);
            input0.analogActions.push_back(ANALOG_ACTION_1);
            input0.analogActionsValues.push_back(Vector2(0.03f, 0.15f));

            Input input1;
            input1.digitalActions.push_back(DIGITAL_ACTION_2);
            input1.digitalActions.push_back(DIGITAL_ACTION_1);
            input1.analogActions.push_back(ANALOG_ACTION_2);
            input1.analogActionsValues.push_back(Vector2(0.12f, 0.97f));

            Vector<ActionsSingleComponent::Actions> clientInput = ReplicateServerInput(server, client, { input0, input1 }, 1);
            TEST_VERIFY(clientInput.size() == 3);
            TEST_VERIFY(clientInput[0].digitalActions.size() == 2);
            TEST_VERIFY(clientInput[0].digitalActions[0] == DIGITAL_ACTION_1);
            TEST_VERIFY(clientInput[0].digitalActions[1] == DIGITAL_ACTION_2);
            TEST_VERIFY(clientInput[0].analogActions.size() == 1);
            VerifyAnalogAction(clientInput[0], ANALOG_ACTION_1, Vector2(0.03f, 0.15f));
            TEST_VERIFY(clientInput[1].digitalActions.size() == 2);
            TEST_VERIFY(clientInput[1].digitalActions[0] == DIGITAL_ACTION_2);
            TEST_VERIFY(clientInput[1].digitalActions[1] == DIGITAL_ACTION_1);
            TEST_VERIFY(clientInput[1].analogActions.size() == 1);
            VerifyAnalogAction(clientInput[1], ANALOG_ACTION_2, Vector2(0.12f, 0.97f));
            TEST_VERIFY(clientInput[2].digitalActions.size() == 2);
            TEST_VERIFY(clientInput[2].digitalActions[0] == DIGITAL_ACTION_2);
            TEST_VERIFY(clientInput[2].digitalActions[1] == DIGITAL_ACTION_1);
            TEST_VERIFY(clientInput[2].analogActions.size() == 1);
            VerifyAnalogAction(clientInput[2], ANALOG_ACTION_2, Vector2(0.12f, 0.97f));

            // Check correction for incorrectly predicted frame above

            Input input2;
            input2.digitalActions.push_back(DIGITAL_ACTION_1);

            clientInput = ReplicateServerInput(server, client, { input2 }, 0, false);
            TEST_VERIFY(clientInput.size() == 2);
            TEST_VERIFY(clientInput[0].digitalActions.size() == 1);
            TEST_VERIFY(clientInput[0].digitalActions[0] == DIGITAL_ACTION_1);
            TEST_VERIFY(clientInput[1].digitalActions.size() == 1);
            TEST_VERIFY(clientInput[1].digitalActions[0] == DIGITAL_ACTION_1);
        }

        // Two frames - two actions per frame in first, no input in second, lots of predicted
        {
            Input input0;
            input0.digitalActions.push_back(DIGITAL_ACTION_1);
            input0.digitalActions.push_back(DIGITAL_ACTION_2);

            Input input1;

            Vector<ActionsSingleComponent::Actions> clientInput = ReplicateServerInput(server, client, { input0, input1 }, 212);
            TEST_VERIFY(clientInput.size() == 1);
            TEST_VERIFY(clientInput[0].digitalActions.size() == 2);
            TEST_VERIFY(clientInput[0].digitalActions[0] == DIGITAL_ACTION_1);
            TEST_VERIFY(clientInput[0].digitalActions[1] == DIGITAL_ACTION_2);
            TEST_VERIFY(clientInput[0].analogActions.size() == 0);
        }

        // Two frames - multiple actions per frame with duplicates, two predicted frames
        {
            Input input0;
            input0.digitalActions.push_back(DIGITAL_ACTION_2);
            input0.digitalActions.push_back(DIGITAL_ACTION_1);
            input0.digitalActions.push_back(DIGITAL_ACTION_2);

            Input input1;
            input1.digitalActions.push_back(DIGITAL_ACTION_1);
            input1.digitalActions.push_back(DIGITAL_ACTION_2);
            input1.digitalActions.push_back(DIGITAL_ACTION_2);
            input1.digitalActions.push_back(DIGITAL_ACTION_1);
            input1.digitalActions.push_back(DIGITAL_ACTION_2);

            Vector<ActionsSingleComponent::Actions> clientInput = ReplicateServerInput(server, client, { input0, input1 }, 2);
            TEST_VERIFY(clientInput.size() == 4);
            TEST_VERIFY(clientInput[0].digitalActions.size() == 2);
            TEST_VERIFY(clientInput[0].digitalActions[0] == DIGITAL_ACTION_2);
            TEST_VERIFY(clientInput[0].digitalActions[1] == DIGITAL_ACTION_1);
            TEST_VERIFY(clientInput[0].analogActions.size() == 0);
            TEST_VERIFY(clientInput[1].digitalActions.size() == 2);
            TEST_VERIFY(clientInput[1].digitalActions[0] == DIGITAL_ACTION_1);
            TEST_VERIFY(clientInput[1].digitalActions[1] == DIGITAL_ACTION_2);
            TEST_VERIFY(clientInput[1].analogActions.size() == 0);
            TEST_VERIFY(clientInput[2].digitalActions.size() == 2);
            TEST_VERIFY(clientInput[2].digitalActions[0] == DIGITAL_ACTION_1);
            TEST_VERIFY(clientInput[2].digitalActions[1] == DIGITAL_ACTION_2);
            TEST_VERIFY(clientInput[2].analogActions.size() == 0);
            TEST_VERIFY(clientInput[3].digitalActions.size() == 2);
            TEST_VERIFY(clientInput[3].digitalActions[0] == DIGITAL_ACTION_1);
            TEST_VERIFY(clientInput[3].digitalActions[1] == DIGITAL_ACTION_2);
            TEST_VERIFY(clientInput[3].analogActions.size() == 0);
        }

        // Tons of frames - to check that client discards most of it and only uses the last ones
        {
            const uint32 NUM_IGNORED_FRAMES = 5000;
            const uint32 NUM_USED_FRAMES = 10;

            Vector<Input> inputs;
            inputs.reserve(NUM_IGNORED_FRAMES + NUM_USED_FRAMES);

            for (uint32 i = 0; i < NUM_IGNORED_FRAMES; ++i)
            {
                Input input;
                input.digitalActions.push_back(DIGITAL_ACTION_1);
                input.analogActions.push_back(ANALOG_ACTION_2);
                input.analogActionsValues.push_back(Vector2(0.44f, 1.0f));

                inputs.push_back(input);
            }

            float32 analogStep = 1.0f / static_cast<float32>(NUM_USED_FRAMES);

            for (uint32 i = 0; i < NUM_USED_FRAMES; ++i)
            {
                Input input;
                input.digitalActions.push_back(DIGITAL_ACTION_2);
                input.analogActions.push_back(ANALOG_ACTION_1);
                input.analogActionsValues.push_back(Vector2(analogStep * i, 0.35f));
                input.analogActions.push_back(ANALOG_ACTION_2);
                input.analogActionsValues.push_back(Vector2(0.51f, analogStep * i));

                inputs.push_back(input);
            }

            Vector<ActionsSingleComponent::Actions> clientInput = ReplicateServerInput(server, client, inputs, 0);
            TEST_VERIFY(clientInput.size() == NetworkRemoteInputComponent::ACTIONS_BUFFER_SIZE);
            for (uint32 i = 0; i < NetworkRemoteInputComponent::ACTIONS_BUFFER_SIZE; ++i)
            {
                TEST_VERIFY(clientInput[i].digitalActions.size() == 1);
                TEST_VERIFY(clientInput[i].digitalActions[0] == DIGITAL_ACTION_2);

                uint32 j = NUM_USED_FRAMES - NetworkRemoteInputComponent::ACTIONS_BUFFER_SIZE + i;
                TEST_VERIFY(clientInput[i].analogActions.size() == 2);
                VerifyAnalogAction(clientInput[i], ANALOG_ACTION_1, Vector2(analogStep * j, 0.35f));
                VerifyAnalogAction(clientInput[i], ANALOG_ACTION_2, Vector2(0.51f, analogStep * j));
            }
        }
    }
};

const DAVA::FastName NetworkRemoteInputSystemTest::DIGITAL_ACTION_1 = DAVA::FastName("DIGITAL_ACTION_1");
const DAVA::FastName NetworkRemoteInputSystemTest::DIGITAL_ACTION_2 = DAVA::FastName("DIGITAL_ACTION_2");
const DAVA::FastName NetworkRemoteInputSystemTest::ANALOG_ACTION_1 = DAVA::FastName("ANALOG_ACTION_1");
const DAVA::FastName NetworkRemoteInputSystemTest::ANALOG_ACTION_2 = DAVA::FastName("ANALOG_ACTION_2");
