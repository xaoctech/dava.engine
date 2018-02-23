#include "NetworkTimeSystem.h"

#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/NetworkTypes.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h"
#include "NetworkCore/UDPTransport/UDPClient.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

#include <Debug/ProfilerCPU.h>
#include <Debug/ProfilerOverlay.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/ActionComponent.h>
#include <Scene3D/Scene.h>
#include <Time/SystemTimer.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkTimeSystem)
{
    ReflectionRegistrator<NetworkTimeSystem>::Begin()[M::Tags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &NetworkTimeSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 17.0f)]
    .Method("ProcessFixed", &NetworkTimeSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 4.0f)]
    .End();
}

static bool IsServerSynced(const Responder& responder, float32 lossFactor, uint32 serverFrameId, uint32 clientFrameId, int32& diff)
{
    const uint32 losses = static_cast<uint32>(std::round(responder.GetPacketLoss() / lossFactor));
    const uint32 buffMinSize = losses + 1; // 1 - artificial latency
    diff = static_cast<int32>(clientFrameId - serverFrameId) - buffMinSize;
    return std::abs(diff) <= static_cast<int32>(NetworkTimeSingleComponent::FrequencyHz / 2);
}

static int8 GetNetDiff(DAVA::NetworkTimeSingleComponent* netTimeComp, const DAVA::Responder& responder)
{
    int32 clientServerDiff = netTimeComp->GetClientServerDiff(responder.GetToken());
    int32 max = std::numeric_limits<int8>::max();
    int32 min = std::numeric_limits<int8>::min();
    return static_cast<int8>(DAVA::Max(DAVA::Min(clientServerDiff, max), min));
}

static void SendUptime(DAVA::NetworkTimeSingleComponent* netTimeComp, const DAVA::Responder& responder)
{
    TimeSyncHeader timeSyncHeader;
    timeSyncHeader.uptimeMs = netTimeComp->GetUptimeMs() + responder.GetRtt() / 2;
    timeSyncHeader.type = TimeSyncHeader::Type::UPTIME;
    timeSyncHeader.netDiff = GetNetDiff(netTimeComp, responder);
    responder.Send(reinterpret_cast<const uint8*>(&timeSyncHeader), TIMESYNC_PACKET_HEADER_SIZE,
                   PacketParams::Reliable(PacketParams::TIME_CHANNEL_ID));
}

NetworkTimeSystem::NetworkTimeSystem(Scene* scene)
    : SceneSystem(scene, 0)
    , fpsMeter(10.f)
{
    scene->SetConstantUpdateTime(NetworkTimeSingleComponent::FrameDurationS);
    scene->SetFixedUpdateTime(NetworkTimeSingleComponent::FrameDurationS);

    if (IsClient(this))
    {
        client = scene->GetSingletonComponent<NetworkClientSingleComponent>()->GetClient();
        client->SubscribeOnConnect(OnClientConnectCb(this, &NetworkTimeSystem::OnConnectClient));
        client->SubscribeOnReceive(PacketParams::TIME_CHANNEL_ID,
                                   OnClientReceiveCb(this, &NetworkTimeSystem::OnReceiveClient));
    }
    else if (IsServer(this))
    {
        IServer* server = scene->GetSingletonComponent<NetworkServerSingleComponent>()->GetServer();
        server->SubscribeOnConnect(OnServerConnectCb(this, &NetworkTimeSystem::OnConnectServer));
        server->SubscribeOnReceive(PacketParams::TIME_CHANNEL_ID,
                                   OnServerReceiveCb(this, &NetworkTimeSystem::OnReceiveServer));
        NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
        netTimeComp->SetIsInitialized(true);
    }
    else
    {
        DVASSERT(false);
    }
}

void NetworkTimeSystem::SetSlowDownFactor(float32 value)
{
    slowDownFactor = value;
}

void NetworkTimeSystem::SetLossFactor(float32 value)
{
    lossFactor = value;
}

const ComponentMask& NetworkTimeSystem::GetResimulationComponents() const
{
    return SceneSystem::GetRequiredComponents();
}

void NetworkTimeSystem::ReSimulationStart(Entity*, uint32 frameId)
{
    DVASSERT(!realCurrFrameId);
    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    realCurrFrameId = netTimeComp->GetFrameId();
    netTimeComp->SetFrameId(frameId);
}

void NetworkTimeSystem::Simulate(Entity*)
{
    DVASSERT(realCurrFrameId);
    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    netTimeComp->SetFrameId(netTimeComp->GetFrameId() + 1);
}

void NetworkTimeSystem::ReSimulationEnd(Entity*)
{
    DVASSERT(realCurrFrameId);
    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    netTimeComp->SetFrameId(realCurrFrameId);
    realCurrFrameId = 0;
}

void NetworkTimeSystem::OnConnectClient()
{
    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    netTimeComp->SetIsInitialized(false);
}

void NetworkTimeSystem::OnConnectServer(const Responder& responder)
{
    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    SendUptime(netTimeComp, responder);
}

void NetworkTimeSystem::OnReceiveClient(const uint8* data, size_t, uint8 channelId, uint32)
{
    const TimeSyncHeader* timeSyncHeader = reinterpret_cast<const TimeSyncHeader*>(data);
    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    netTimeComp->SetClientServerDiff(client->GetAuthToken(), timeSyncHeader->netDiff);
    switch (timeSyncHeader->type)
    {
    case TimeSyncHeader::Type::UPTIME:
    {
        netTimeComp->SetUptimeMs(timeSyncHeader->uptimeMs);
        netTimeComp->SetFrameId(timeSyncHeader->uptimeMs / NetworkTimeSingleComponent::FrameDurationMs);
        netTimeComp->SetIsInitialized(true);
        break;
    }
    case TimeSyncHeader::Type::DIFF:
    {
        ProcessFrameDiff(timeSyncHeader->diff);
        break;
    }
    default:
        break;
    }
}

void NetworkTimeSystem::OnReceiveServer(const Responder& responder, const uint8* data, size_t)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkTimeSystem::OnReceiveServer");
    const TimeSyncHeader* inHeader = reinterpret_cast<const TimeSyncHeader*>(data);
    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    uint32 lastClientFramId = netTimeComp->GetLastClientFrameId(responder.GetToken());
    if (inHeader->frameId <= lastClientFramId)
    { // is too late
        return;
    }
    netTimeComp->SetLastClientFrameId(responder.GetToken(), inHeader->frameId);

    int32 diff = 0;
    if (IsServerSynced(responder, lossFactor, netTimeComp->GetFrameId(), inHeader->frameId, diff))
    {
        TimeSyncHeader outHeader;
        outHeader.type = TimeSyncHeader::Type::DIFF;
        outHeader.diff = diff;
        outHeader.netDiff = GetNetDiff(netTimeComp, responder);
        responder.Send(reinterpret_cast<const uint8*>(&outHeader), TIMESYNC_PACKET_HEADER_SIZE,
                       PacketParams::Unreliable(PacketParams::TIME_CHANNEL_ID));
    }
    else
    {
        SendUptime(netTimeComp, responder);
    }
}

void NetworkTimeSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkTimeSystem::ProcessFixed");
    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();

    uint32 uptime = netTimeComp->GetUptimeMs();
    uint32 frameId = netTimeComp->GetFrameId();

    if (client)
    {
        int32 waitFrames = netTimeComp->GetWaitFrames();
        if (waitFrames == 0)
        {
            GetScene()->SetFixedUpdateTime(NetworkTimeSingleComponent::FrameDurationS);
        }
        else
        {
            if (waitFrames > 0)
            {
                GetScene()->SetFixedUpdateTime(NetworkTimeSingleComponent::FrameDurationS + NetworkTimeSingleComponent::FrameAccelerationS);
                netTimeComp->SetWaitFrames(waitFrames - 1);
            }
            if (waitFrames < 0)
            {
                GetScene()->SetFixedUpdateTime(NetworkTimeSingleComponent::FrameDurationS - NetworkTimeSingleComponent::FrameAccelerationS);
                netTimeComp->SetWaitFrames(waitFrames + 1);
            }
        }

        TimeSyncHeader timeSyncHeader;
        timeSyncHeader.type = TimeSyncHeader::Type::FRAME;
        timeSyncHeader.frameId = netTimeComp->GetFrameId();
        timeSyncHeader.netDiff = 0;
        client->Send(reinterpret_cast<const uint8*>(&timeSyncHeader), TIMESYNC_PACKET_HEADER_SIZE,
                     PacketParams::Unreliable(PacketParams::TIME_CHANNEL_ID));
    }

    netTimeComp->SetUptimeMs(uptime + static_cast<uint32>(timeElapsed * 1000));
    netTimeComp->SetFrameId(frameId + 1);
}

void NetworkTimeSystem::Process(float32 timeElapsed)
{
    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    fpsMeter.Update(timeElapsed);
    if (fpsMeter.IsFpsReady())
    {
        netTimeComp->SetFps(fpsMeter.GetFps());
        Logger::Debug("[NetworkTimeSystem] FPS %.2f", fpsMeter.GetFps());
    }
    ProfilerOverlay* overlay = ProfilerOverlay::globalProfilerOverlay;
    overlay->AddCustomMetric(FastName("FPS"), netTimeComp->GetFps());
    if (client)
    {
        overlay->AddCustomMetric(FastName("Wait Frames"), netTimeComp->GetWaitFrames());
        overlay->AddCustomMetric(FastName("Frame ID"), netTimeComp->GetFrameId());
    }
}

void NetworkTimeSystem::ProcessFrameDiff(int32 diff)
{
    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    if (diff != 0 && netTimeComp->GetWaitFrames() == 0)
    {
        int32 waitFrames = diff * static_cast<int32>(NetworkTimeSingleComponent::FrameDurationS / NetworkTimeSingleComponent::FrameAccelerationS);
        if (diff < 0)
        {
            netTimeComp->SetWaitFrames(waitFrames);
        }
        else
        {
            waitFrames = static_cast<int32>(static_cast<float32>(waitFrames) * slowDownFactor);
            waitFrames = diff == 1 ? std::max(waitFrames, 1) : waitFrames;
            netTimeComp->SetWaitFrames(waitFrames);
        }
    }
}
}
