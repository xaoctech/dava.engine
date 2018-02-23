#include "GameClient.h"

#include "Components/SingleComponents/GameModeSingleComponent.h"
#include "Systems/ShootSystem.h"

#include <Concurrency/Thread.h>
#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Time/SystemTimer.h>

#include <NetworkCore/Private/NetworkSerialization.h>
#include <NetworkCore/Scene3D/Systems/NetworkInputSystem.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>

namespace GameClientDetail
{
static const DAVA::int32 MAX_UPDATE_DURATION_US = 2000;
}

DAVA_REFLECTION_IMPL(GameClient)
{
    DAVA::ReflectionRegistrator<GameClient>::Begin()
    .Field("udpClient", &GameClient::GetUDPClientPtr, nullptr)
    .End();
}

GameClient::GameClient(const DAVA::String& hostName, DAVA::uint16 port, const DAVA::FastName& token)
    : udpClient(hostName, port, token, 1)
{
    using namespace DAVA;
    udpClient.SubscribeOnConnect(OnClientConnectCb(this, &GameClient::OnConnect));
    udpClient.SubscribeOnDisconnect(OnClientDisconnectCb(this, &GameClient::OnDisconnect));
}

void GameClient::Update(DAVA::float32 timeElapsed)
{
    using namespace DAVA;
    DAVA_PROFILER_CPU_SCOPE("GameClient::Update")
    using namespace GameClientDetail;

    int64 updUs = SystemTimer::GetUs();
    while (udpClient.Update())
    { // limit
        if (SystemTimer::GetUs() - updUs > MAX_UPDATE_DURATION_US)
        {
            break;
        }
    }
    if (opts.isBot)
    {
        int64 delta = 0;
        if (frameTimeUs > 0)
        {
            delta = SystemTimer::GetUs() - frameTimeUs;
        }
        int64 spinDurUs = NetworkTimeSingleComponent::FrameDurationUs - delta;
        while (spinDurUs > MAX_UPDATE_DURATION_US)
        {
            int64 sleepUs = SystemTimer::GetUs();
            Thread::Sleep(1);
            spinDurUs -= (SystemTimer::GetUs() - sleepUs);
        }
        int64 sleepUs = SystemTimer::GetUs();
        while (SystemTimer::GetUs() - sleepUs < spinDurUs)
            ;
        frameTimeUs = SystemTimer::GetUs();
    }

    if (opts.scene)
    {
        GameModeSingleComponent* gameModeComp = opts.scene->GetSingletonComponent<GameModeSingleComponent>();
        if (gameModeComp != nullptr && gameModeComp->GetPlayer() != nullptr)
        {
            serviceInfo.entityId = gameModeComp->GetPlayer()->GetID();
        }
    }
}

void GameClient::Setup(const Options& opts_)
{
    opts = opts_;
}

void GameClient::OnConnect()
{
    serviceInfo.state = State::Connected;
}

void GameClient::OnDisconnect()
{
    serviceInfo.state = State::Idle;
}

DAVA::UDPClient& GameClient::GetUDPClient()
{
    return udpClient;
}
