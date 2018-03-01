#include "NetworkTimeSystem.h"

#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/NetworkTypes.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkResimulationSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
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
    .Method("ProcessFixed", &NetworkTimeSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 3.0f)]
    .End();
}

static bool IsServerSynced(const Responder& responder, uint32 serverFrameId, uint32 clientFrameId, int32& diff)
{
    const uint32 losses = static_cast<uint32>(std::round(responder.GetPacketLoss() / NetworkTimeSingleComponent::LossFactor));
    diff = static_cast<int32>(clientFrameId - serverFrameId) - losses - NetworkTimeSingleComponent::ArtificialLatency;
    return std::abs(diff) <= static_cast<int32>(NetworkTimeSingleComponent::FrequencyHz / 2);
}

static int8 GetNetDiff(DAVA::NetworkTimeSingleComponent* netTimeComp, const DAVA::Responder& responder)
{
    int32 clientServerDiff = netTimeComp->GetClientOutrunning(responder.GetToken());
    int32 max = std::numeric_limits<int8>::max();
    int32 min = std::numeric_limits<int8>::min();
    return static_cast<int8>(DAVA::Max(DAVA::Min(clientServerDiff, max), min));
}

static void SendUptime(DAVA::NetworkTimeSingleComponent* netTimeComp, const DAVA::Responder& responder)
{
    TimeSyncHeader timeSyncHeader;
    timeSyncHeader.uptimeMs = netTimeComp->GetUptimeMs() +
    static_cast<uint32>(responder.GetRtt() * NetworkTimeSingleComponent::UptimeInitFactor);
    timeSyncHeader.type = TimeSyncHeader::Type::UPTIME;
    timeSyncHeader.netDiff = GetNetDiff(netTimeComp, responder);
    responder.Send(reinterpret_cast<const uint8*>(&timeSyncHeader), TIMESYNC_PACKET_HEADER_SIZE,
                   PacketParams::Reliable(PacketParams::TIME_CHANNEL_ID));

    uint32 resyncDelayUptime = netTimeComp->GetUptimeMs() +
    static_cast<uint32>(responder.GetRtt() * NetworkTimeSingleComponent::UptimeDelayFactor);
    netTimeComp->SetResyncDelayUptime(responder.GetToken(), resyncDelayUptime);
}

NetworkTimeSystem::NetworkTimeSystem(Scene* scene)
    : BaseSimulationSystem(scene, 0)
    , fpsMeter(10.f)
{
    netTimeComp = scene->GetSingletonComponent<NetworkTimeSingleComponent>();

    scene->SetConstantUpdateTime(NetworkTimeSingleComponent::FrameDurationS);
    scene->SetFixedUpdateTime(NetworkTimeSingleComponent::FrameDurationS);

    if (IsClient(this))
    {
        client = scene->GetSingletonComponent<NetworkClientSingleComponent>()->GetClient();
        client->SubscribeOnConnect(OnClientConnectCb(this, &NetworkTimeSystem::OnConnectClient));
        client->SubscribeOnReceive(PacketParams::TIME_CHANNEL_ID,
                                   OnClientReceiveCb(this, &NetworkTimeSystem::OnReceiveClient));
        networkResimulationSingleComponent = scene->AquireSingleComponentForRead<NetworkResimulationSingleComponent>();
    }
    else if (IsServer(this))
    {
        IServer* server = scene->GetSingletonComponent<NetworkServerSingleComponent>()->GetServer();
        server->SubscribeOnConnect(OnServerConnectCb(this, &NetworkTimeSystem::OnConnectServer));
        server->SubscribeOnReceive(PacketParams::TIME_CHANNEL_ID,
                                   OnServerReceiveCb(this, &NetworkTimeSystem::OnReceiveServer));
        netTimeComp->SetIsInitialized(true);
    }
    else
    {
        DVASSERT(false);
    }
}

void NetworkTimeSystem::ReSimulationStart()
{
    DVASSERT(realCurrFrameId == 0);
    realCurrFrameId = netTimeComp->GetFrameId();

    uint32 frameId = networkResimulationSingleComponent->GetResimulationFrameId();
    netTimeComp->SetFrameId(frameId);
}

void NetworkTimeSystem::ReSimulationEnd()
{
    DVASSERT(realCurrFrameId != 0);
    netTimeComp->SetFrameId(realCurrFrameId);
    realCurrFrameId = 0;
}

void NetworkTimeSystem::OnConnectClient()
{
    netTimeComp->SetIsInitialized(false);
}

void NetworkTimeSystem::OnConnectServer(const Responder& responder)
{
    SendUptime(netTimeComp, responder);
}

void NetworkTimeSystem::OnReceiveClient(const uint8* data, size_t, uint8 channelId, uint32)
{
    const TimeSyncHeader* timeSyncHeader = reinterpret_cast<const TimeSyncHeader*>(data);

    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    netTimeComp->SetClientOutrunning(client->GetAuthToken(), timeSyncHeader->netDiff);
    netTimeComp->SetLastSyncDiff(timeSyncHeader->diff);

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
    uint32 lastClientFrameId = netTimeComp->GetLastClientFrameId(responder.GetToken());
    if (inHeader->frameId <= lastClientFrameId)
    { // is too late
        return;
    }
    netTimeComp->SetLastClientFrameId(responder.GetToken(), inHeader->frameId);
    netTimeComp->SetClientViewDelay(responder.GetToken(), inHeader->frameId, inHeader->netDiff);

    if (netTimeComp->GetUptimeMs() < netTimeComp->GetResyncDelayUptime(responder.GetToken()))
    { // wait till resync reaches client
        return;
    }

    int32 diff = 0;
    if (IsServerSynced(responder, netTimeComp->GetFrameId(), inHeader->frameId, diff))
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

    uint32 frameId = netTimeComp->GetFrameId();
    ++frameId;
    netTimeComp->SetFrameId(frameId);

    if (IsReSimulating())
    {
        DVASSERT(realCurrFrameId);
        return;
    }

    uint32 uptime = netTimeComp->GetUptimeMs();
    uptime += static_cast<uint32>(timeElapsed * 1000);
    netTimeComp->SetUptimeMs(uptime);

    if (client)
    {
        int32 adjustedFrames = netTimeComp->GetAdjustedFrames();
        if (adjustedFrames == 0)
        {
            GetScene()->SetFixedUpdateTime(NetworkTimeSingleComponent::FrameDurationS);
        }
        else
        {
            if (adjustedFrames > 0)
            {
                // client is ahead of server - slowing down
                GetScene()->SetFixedUpdateTime(NetworkTimeSingleComponent::FrameDurationS + NetworkTimeSingleComponent::FrameSlowdownS);
                netTimeComp->SetAdjustedFrames(adjustedFrames - 1);
            }
            if (adjustedFrames < 0)
            {
                // client is behind server - speeding up
                GetScene()->SetFixedUpdateTime(NetworkTimeSingleComponent::FrameDurationS - NetworkTimeSingleComponent::FrameSpeedupS);
                netTimeComp->SetAdjustedFrames(adjustedFrames + 1);
            }
        }

        TimeSyncHeader timeSyncHeader;
        timeSyncHeader.type = TimeSyncHeader::Type::FRAME;
        timeSyncHeader.frameId = frameId;
        timeSyncHeader.netDiff = static_cast<int8>(frameId - netTimeComp->GetLastServerFrameId());
        client->Send(reinterpret_cast<const uint8*>(&timeSyncHeader), TIMESYNC_PACKET_HEADER_SIZE,
                     PacketParams::Unreliable(PacketParams::TIME_CHANNEL_ID));
    }
}

void NetworkTimeSystem::Process(float32 timeElapsed)
{
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
        overlay->AddCustomMetric(FastName("Wait Frames"), netTimeComp->GetAdjustedFrames());
        overlay->AddCustomMetric(FastName("Frame ID"), netTimeComp->GetFrameId());
    }
}

void NetworkTimeSystem::ProcessFrameDiff(int32 diff)
{
    if (diff == 0)
    { // perfectly synced
        return;
    }

    NetworkTimeSingleComponent* netTimeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    int32 adjustedFrames = netTimeComp->GetAdjustedFrames();
    bool sameDirection = (diff > 0) ^ (adjustedFrames < 0);

    if (adjustedFrames == 0 || (diff < 0 && !sameDirection))
    { // starting over or changing direction to speedup
        float32 deltaS = diff < 0 ? NetworkTimeSingleComponent::FrameSpeedupS : NetworkTimeSingleComponent::FrameSlowdownS;
        adjustedFrames = static_cast<int32>(std::round(diff * NetworkTimeSingleComponent::FrameDurationS / deltaS));
        netTimeComp->SetAdjustedFrames(adjustedFrames);
    }
}
}
