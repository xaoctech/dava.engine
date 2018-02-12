#include "NetworkGameModeSystem.h"

#include "NetworkCore/NetworkTypes.h"
#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>
#include <Entity/ComponentUtils.h>

#include <memory>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkGameModeSystem)
{
    ReflectionRegistrator<NetworkGameModeSystem>::Begin()[M::Tags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &NetworkGameModeSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 1.0f)]
    .End();
}

NetworkGameModeSystem::NetworkGameModeSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkPlayerComponent>() | ComponentUtils::MakeMask<NetworkReplicationComponent>())
{
    if (IsServer(this))
    {
        server = scene->GetSingletonComponent<NetworkServerSingleComponent>()->GetServer();
        server->SubscribeOnTokenConfirmation(OnServerTokenConfirmationCb(this, &NetworkGameModeSystem::OnTokenConfirmationServer));
        server->SubscribeOnDisconnect(OnServerDisconnectCb(this, &NetworkGameModeSystem::OnDisconnectServer));
        server->SubscribeOnReceive(PacketParams::GAMEMODE_CHANNEL_ID, OnServerReceiveCb(this, &NetworkGameModeSystem::OnReceiveServer));
    }
    else if (IsClient(this))
    {
        client = scene->GetSingletonComponent<NetworkClientSingleComponent>()->GetClient();
        client->SubscribeOnReceive(PacketParams::GAMEMODE_CHANNEL_ID, OnClientReceiveCb(this, &NetworkGameModeSystem::OnReceiveClient));
    }
    else
    {
        DVASSERT(false);
    }

    netGameModeComp = scene->GetSingletonComponent<NetworkGameModeSingleComponent>();
    actionSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();
}

void NetworkGameModeSystem::AddEntity(Entity* entity)
{
    NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
    netGameModeComp->AddPlayerEnity(netReplComp->GetNetworkPlayerID(), entity);
}

void NetworkGameModeSystem::RemoveEntity(Entity* entity)
{
    NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
    netGameModeComp->RemovePlayerEntity(netReplComp->GetNetworkPlayerID());
}

void NetworkGameModeSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkGameModeSystem::Process");

    if (server)
    {
        netGameModeComp->ClearConnectedTokens();
        netGameModeComp->ClearLoadedTokens();
    }
    if (client)
    {
        if (netGameModeComp->IsLoaded())
        {
            GameModePacketHeader header;
            header.isLoaded = true;
            PacketParams params = PacketParams::Reliable(PacketParams::GAMEMODE_CHANNEL_ID);
            client->Send(reinterpret_cast<const uint8*>(&header), GAMEMODE_PACKET_HEADER_SIZE, params);
            netGameModeComp->SetIsLoaded(false);
        }
    }
}

void NetworkGameModeSystem::OnTokenConfirmationServer(const Responder& responder)
{
    const FastName& token = responder.GetToken();
    const UnorderedSet<FastName>& validTokens = netGameModeComp->GetValidTokens();
    bool isValidToken = validTokens.find(token) != validTokens.end();
#ifndef NDEBUG
    isValidToken = true;
    NetworkPlayerID playerID = netGameModeComp->GetNextNetworkPlayerID();
    netGameModeComp->AddNetworkPlayerID(token, playerID);
#endif
    if (isValidToken)
    {
        netGameModeComp->AddConnectedToken(token);
        server->SetValidToken(token);
        NetworkPlayerID playerID = netGameModeComp->GetNetworkPlayerID(token);
        GameModePacketHeader header;
        header.networkPlayerID = playerID;
        PacketParams params = PacketParams::Reliable(PacketParams::GAMEMODE_CHANNEL_ID);
        responder.Send(reinterpret_cast<const uint8*>(&header), GAMEMODE_PACKET_HEADER_SIZE, params);
    }
    else
    {
        server->Disconnect(token);
    }
}

void NetworkGameModeSystem::OnDisconnectServer(const FastName& token)
{
    netGameModeComp->RemoveConnectedToken(token);
}

void NetworkGameModeSystem::OnReceiveServer(const Responder& responder, const uint8* data, size_t size)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkGameModeSystem::OnReceiveServer");
    const GameModePacketHeader* header = reinterpret_cast<const GameModePacketHeader*>(data);
    if (header->isLoaded)
    {
        netGameModeComp->AddReadyToken(responder.GetToken());
    }
}

void NetworkGameModeSystem::OnReceiveClient(const uint8* data, size_t, uint8, uint32)
{
    const GameModePacketHeader* header = reinterpret_cast<const GameModePacketHeader*>(data);
    if (header->networkPlayerID)
    {
        netGameModeComp->SetNetworkPlayerID(header->networkPlayerID);
        actionSingleComponent->SetLocalPlayerId(header->networkPlayerID);
    }
}
}
