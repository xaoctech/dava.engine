#include "NetworkHealthCheckSystem.h"

#include "NetworkCore/UDPTransport/UDPClient.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Time/SystemTimer.h>

namespace DAVA
{
namespace NetworkHealthCheckSystemDetail
{
static const int64 MAX_SPIN_DURATION_US = 10;
static const uint16 HEARTBEAT_CODE = 200;
static const size_t HEARTBEAT_CODE_SIZE = sizeof(uint16);
}

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkHealthCheckSystem)
{
    ReflectionRegistrator<NetworkHealthCheckSystem>::Begin()[M::Tags("network", "server")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &NetworkHealthCheckSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 1000.0f)]
    .End();
}

NetworkHealthCheckSystem::NetworkHealthCheckSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
}

void NetworkHealthCheckSystem::Connect(const DAVA::String& host, uint16 port)
{
    client.reset(new UDPClient(host, port, FastName(), 1, 10));
}

void NetworkHealthCheckSystem::Process(float32 timeElapsed)
{
    using namespace NetworkHealthCheckSystemDetail;
    DAVA_PROFILER_CPU_SCOPE("NetworkHealthCheckSystem::Process");
    if (client)
    {
        if (client->IsConnected())
        {
            heartbeatTimeout += timeElapsed;
            if (heartbeatTimeout > maxHeartbeatTimeout)
            {
                client->Send(reinterpret_cast<const uint8*>(&HEARTBEAT_CODE), HEARTBEAT_CODE_SIZE,
                             PacketParams::Reliable(PacketParams::SERVICE_CHANNEL_ID));
                heartbeatTimeout -= maxHeartbeatTimeout;
            }
        }
        int64 beginUs = SystemTimer::GetUs();
        while (client->Update() && SystemTimer::GetUs() - beginUs < MAX_SPIN_DURATION_US)
            ;
    }
}
}
