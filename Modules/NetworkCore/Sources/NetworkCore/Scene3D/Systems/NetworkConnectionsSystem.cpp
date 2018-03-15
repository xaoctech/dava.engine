#include "NetworkConnectionsSystem.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Scene.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkConnectionsSingleComponent.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkConnectionsSystem)
{
    ReflectionRegistrator<NetworkConnectionsSystem>::Begin()[M::Tags("network", "server")]
    .ConstructorByPointer<Scene*>()
    .End();
}

NetworkConnectionsSystem::NetworkConnectionsSystem(Scene* scene)
    : SceneSystem(scene, ComponentMask())
{
    server = scene->GetSingleComponent<NetworkServerSingleComponent>()->GetServer();
    DVASSERT(server);
    networkConnections = scene->GetSingleComponent<NetworkConnectionsSingleComponent>();
    DVASSERT(networkConnections);
    server->SubscribeOnConnect([this](const Responder& responder) {
        networkConnections->AddConnectedToken(responder.GetToken());
    });
    server->SubscribeOnDisconnect([this](const FastName& token) {
        networkConnections->RemoveConnectedToken(token);
    });
    for (uint8 i = 0; i < PacketParams::CHANNELS_COUNT; ++i)
    {
        server->SubscribeOnReceive(i, [this, i](const Responder& responder, const uint8* data, size_t size) {
            networkConnections->StoreRecvPacket(i, responder.GetToken(), data, size);
        });
    }
}
}
