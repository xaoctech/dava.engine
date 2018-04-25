#include "NetworkConnectionsSystem.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Scene.h"
#include "NetworkCore/UDPTransport/UDPServer.h"
#include "NetworkCore/UDPTransport/UDPClient.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerConnectionsSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientConnectionSingleComponent.h"
#include "NetworkCore/NetworkCoreUtils.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkConnectionsSystem)
{
    ReflectionRegistrator<NetworkConnectionsSystem>::Begin()[M::SystemTags("network")]
    .ConstructorByPointer<Scene*>()
    .End();
}

NetworkConnectionsSystem::NetworkConnectionsSystem(Scene* scene)
    : SceneSystem(scene, ComponentMask())
{
    if (IsServer(scene))
    {
        IServer* server = scene->GetSingleComponent<NetworkServerSingleComponent>()->GetServer();
        DVASSERT(server);
        NetworkServerConnectionsSingleComponent* networkConnections = scene->GetSingleComponent<NetworkServerConnectionsSingleComponent>();
        DVASSERT(networkConnections);

        server->SetNetworkEventStorage(*networkConnections);
    }
    else if (IsClient(scene))
    {
        IClient* client = scene->GetSingleComponent<NetworkClientSingleComponent>()->GetClient();
        DVASSERT(client);
        NetworkClientConnectionSingleComponent* networkConnection = scene->GetSingleComponent<NetworkClientConnectionSingleComponent>();
        DVASSERT(networkConnection);
        client->SubscribeOnConnect([networkConnection]() {
            networkConnection->SetIsConnected(true);
            networkConnection->SetIsJustConnected(true);
        });
        client->SubscribeOnDisconnect([networkConnection]() {
            networkConnection->SetIsConnected(false);
            networkConnection->SetIsJustDisconnected(true);
        });
        for (uint8 i = 0; i < PacketParams::CHANNELS_COUNT; ++i)
        {
            client->SubscribeOnReceive(i, [networkConnection, i](const uint8* data, size_t size, uint8, uint32) {
                networkConnection->StoreRecvPacket(i, data, size);
            });
        }
    }
}
}
