#include "NetworkTimelineControlSystem.h"

#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/UDPTransport/UDPClient.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimelineSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkTimelineControlSystem)
{
    ReflectionRegistrator<NetworkTimelineControlSystem>::Begin()[M::Tags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &NetworkTimelineControlSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 18.0f)]
    .Method("ProcessFixed", &NetworkTimelineControlSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 1.0f)]
    .End();
}

NetworkTimelineControlSystem::NetworkTimelineControlSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
    packetHeader.type = ServicePacketHeader::ServiceType::TIMELINE_CONTROL;
    packetHeader.value.timeline = ServicePacketHeader::TimelineControlType::PAUSE;

    if (IsServer(this))
    {
        server = scene->GetSingletonComponent<NetworkServerSingleComponent>()->GetServer();
        server->SubscribeOnConnect(OnServerConnectCb(this, &NetworkTimelineControlSystem::OnConnectServer));
        server->SubscribeOnReceive(PacketParams::SERVICE_CHANNEL_ID,
                                   OnServerReceiveCb(this, &NetworkTimelineControlSystem::OnReceiveServer));
    }
    else if (IsClient(this))
    {
        client = scene->GetSingletonComponent<NetworkClientSingleComponent>()->GetClient();
        client->SubscribeOnReceive(PacketParams::SERVICE_CHANNEL_ID,
                                   OnClientReceiveCb(this, &NetworkTimelineControlSystem::OnReceiveClient));
    }
    else
    {
        DVASSERT(false);
    }

    netTimelineComp = scene->GetSingletonComponent<NetworkTimelineSingleComponent>();
}

void NetworkTimelineControlSystem::Process(float32 timeElapsed)
{
    if (client)
    {
        ProcessClient(netTimelineComp);
    }
    if (server)
    {
        ProcessServer(netTimelineComp);
    }
}

void NetworkTimelineControlSystem::ProcessFixed(float32 timeElapsed)
{
    int32 stepsCount = netTimelineComp->GetStepsCount();
    if (stepsCount > 0)
    {
        --stepsCount;
        if (stepsCount == 0)
        {
            GetScene()->PauseFixedUpdate();
        }
    }
    netTimelineComp->SetStepsCount(stepsCount);
}

void NetworkTimelineControlSystem::OnConnectServer(const Responder& responder)
{
    if (GetScene()->IsFixedUpdatePaused())
    { // the server already was paused, send pause to the new client
        packetHeader.value.timeline = ServicePacketHeader::TimelineControlType::PAUSE;
        responder.Send(reinterpret_cast<const uint8*>(&packetHeader), SERVICE_PACKET_HEADER_SIZE,
                       PacketParams::Reliable(PacketParams::SERVICE_CHANNEL_ID));
    }
}

void NetworkTimelineControlSystem::OnReceiveServer(const Responder& responder, const uint8* data, size_t)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkTimelineControlSystem::OnReceiveServer");
    OnReceive(data);
}

void NetworkTimelineControlSystem::OnReceiveClient(const uint8* data, size_t, uint8, uint32)
{
    OnReceive(data);
}

void NetworkTimelineControlSystem::ProcessClient(NetworkTimelineSingleComponent* netTimelineComp)
{
    if (netTimelineComp->IsClientJustPaused())
    { // pause/unpause the client
        SwitchPauseScene();
        netTimelineComp->SetClientJustPaused(false);
    }

    bool notifyServer = false;
    if (netTimelineComp->IsServerJustPaused())
    { // send pause to the server
        netTimelineComp->SetServerJustPaused(false);
        packetHeader.value.timeline = ServicePacketHeader::TimelineControlType::PAUSE;
        notifyServer = true;
    }
    if (netTimelineComp->HasStepOver())
    {
        netTimelineComp->SetStepOver(false);
        packetHeader.value.timeline = ServicePacketHeader::TimelineControlType::STEP_OVER;
        notifyServer = true;
    }
    if (notifyServer)
    {
        client->Send(reinterpret_cast<const uint8*>(&packetHeader), SERVICE_PACKET_HEADER_SIZE,
                     PacketParams::Reliable(PacketParams::SERVICE_CHANNEL_ID));
    }
}

void NetworkTimelineControlSystem::ProcessServer(NetworkTimelineSingleComponent* netTimelineComp)
{
    bool notifyClients = false;
    if (netTimelineComp->IsServerJustPaused())
    { // pause/unpause the server
        SwitchPauseScene();
        netTimelineComp->SetServerJustPaused(false);
        // send pause or unpase to clients
        packetHeader.value.timeline = GetScene()->IsFixedUpdatePaused()
        ?
        ServicePacketHeader::TimelineControlType::PAUSE
        :
        ServicePacketHeader::TimelineControlType::UNPAUSE;
        notifyClients = true;
    }
    if (netTimelineComp->HasStepOver())
    {
        netTimelineComp->SetStepOver(false);
        StepOver(netTimelineComp);
        packetHeader.value.timeline = ServicePacketHeader::TimelineControlType::STEP_OVER;
        notifyClients = true;
    }
    if (notifyClients)
    {
        server->Broadcast(reinterpret_cast<const uint8*>(&packetHeader), SERVICE_PACKET_HEADER_SIZE,
                          PacketParams::Reliable(PacketParams::SERVICE_CHANNEL_ID));
    }
}

void NetworkTimelineControlSystem::StepOver(NetworkTimelineSingleComponent* netTimelineComp) const
{
    if (GetScene()->IsFixedUpdatePaused())
    {
        GetScene()->UnpauseFixedUpdate();
    }
    netTimelineComp->SetStepsCount(1);
}

void NetworkTimelineControlSystem::SwitchPauseScene() const
{
    Scene* scene = GetScene();
    if (scene->IsFixedUpdatePaused())
    {
        scene->UnpauseFixedUpdate();
    }
    else
    {
        scene->PauseFixedUpdate();
    }
}

void NetworkTimelineControlSystem::OnReceive(const uint8* data) const
{
    const ServicePacketHeader* packetHeader = reinterpret_cast<const ServicePacketHeader*>(data);
    if (packetHeader->type == ServicePacketHeader::ServiceType::TIMELINE_CONTROL)
    {
        switch (packetHeader->value.timeline)
        {
        case ServicePacketHeader::TimelineControlType::PAUSE:
        {
            if (server)
            {
                netTimelineComp->SetServerJustPaused(true);
            }
            if (client && !GetScene()->IsFixedUpdatePaused())
            {
                netTimelineComp->SetClientJustPaused(true);
            }
            break;
        }
        case ServicePacketHeader::TimelineControlType::UNPAUSE:
        {
            if (client && GetScene()->IsFixedUpdatePaused())
            {
                netTimelineComp->SetClientJustPaused(true);
            }
            break;
        }
        case ServicePacketHeader::TimelineControlType::STEP_OVER:
        {
            if (server)
            {
                netTimelineComp->SetStepOver(true);
            }
            if (client)
            {
                StepOver(netTimelineComp);
            }
            break;
        }
        default:
            break;
        }
    }
}
}
