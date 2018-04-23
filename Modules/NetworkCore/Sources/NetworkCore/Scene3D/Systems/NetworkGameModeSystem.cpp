#include "NetworkGameModeSystem.h"

#include "NetworkCore/NetworkTypes.h"
#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerConnectionsSingleComponent.h"

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
    .Method("ProcessFixed", &NetworkGameModeSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 0.7f)]
    .End();
}

NetworkGameModeSystem::NetworkGameModeSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkPlayerComponent>() | ComponentUtils::MakeMask<NetworkReplicationComponent>())
{
    if (IsServer(this))
    {
        netConnectionsComp = scene->GetSingleComponent<NetworkServerConnectionsSingleComponent>();
        DVASSERT(netConnectionsComp);
        server = scene->GetSingleComponentForRead<NetworkServerSingleComponent>(this)->GetServer();
    }
    else if (IsClient(this))
    {
        client = scene->GetSingleComponentForRead<NetworkClientSingleComponent>(this)->GetClient();
        client->SubscribeOnReceive(PacketParams::GAMEMODE_CHANNEL_ID, OnClientReceiveCb(this, &NetworkGameModeSystem::OnReceiveClient));
    }
    else
    {
        DVASSERT(false);
    }

    netGameModeComp = scene->GetSingleComponent<NetworkGameModeSingleComponent>();
    actionSingleComponent = scene->GetSingleComponentForWrite<ActionsSingleComponent>(this);
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

void NetworkGameModeSystem::ProcessFixed(float32 /*timeElapsed*/)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkGameModeSystem::Process");

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
    if (server)
    {
        const Vector<FastName>& confirmedTokens = netConnectionsComp->GetConfirmedTokens();
        for (auto& token : confirmedTokens)
        {
            const Responder& responder = server->GetResponder(token);
            OnTokenConfirmationServer(responder);
        }

        const Vector<NetworkServerConnectionsSingleComponent::ServerRecvPacket>& packets =
        netConnectionsComp->GetRecvPackets(PacketParams::GAMEMODE_CHANNEL_ID);

        for (auto& packet : packets)
        {
            const Responder& responder = server->GetResponder(packet.token);
            OnReceiveServer(responder, packet.data.data(), packet.data.size());
        }

        const auto& disconnected = netConnectionsComp->GetJustDisconnectedTokens();
        for (auto& token : disconnected)
        {
            OnDisconnectServer(token);
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
    NetworkPlayerID playerID = netGameModeComp->GetNetworkPlayerID(token);
    if (playerID == 0)
    {
        playerID = netGameModeComp->GetNextNetworkPlayerID();
        netGameModeComp->AddNetworkPlayerID(token, playerID);
    }
#endif
    if (isValidToken)
    {
        server->SetValidToken(token);
        const NetworkPlayerID playerID_ = netGameModeComp->GetNetworkPlayerID(token);
        GameModePacketHeader header{ playerID_ };
        const PacketParams params = PacketParams::Reliable(PacketParams::GAMEMODE_CHANNEL_ID);
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
        const auto& connectedTokens = netConnectionsComp->GetConnectedTokens();
        DVASSERT(connectedTokens.find(responder.GetToken()) != connectedTokens.end());
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
