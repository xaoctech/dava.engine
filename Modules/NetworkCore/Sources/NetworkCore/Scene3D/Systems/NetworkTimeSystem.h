#pragma once

#include "NetworkCore/UDPTransport/UDPServer.h"
#include <Scene3D/Systems/BaseSimulationSystem.h>

#include <Entity/SceneSystem.h>
#include <Utils/FpsMeter.h>

namespace DAVA
{
class IClient;
class NetworkTimeSingleComponent;
class NetworkResimulationSingleComponent;

class NetworkTimeSystem : public BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkTimeSystem, BaseSimulationSystem);

    NetworkTimeSystem(Scene* scene);

    void ReSimulationStart() override;
    void ReSimulationEnd() override;

    void OnConnectClient();
    void OnConnectServer(const Responder& responder);
    void OnReceiveClient(const uint8* data, size_t, uint8 channelId, uint32);
    void OnReceiveServer(const Responder& responder, const uint8* data, size_t);
    void ProcessFixed(float32 timeElapsed) override;
    void Process(float32 timeElapsed) override;
    void PrepareForRemove() override{};
    void ProcessFrameDiff(int32 diff);

private:
    NetworkTimeSingleComponent* netTimeComp = nullptr;
    const NetworkResimulationSingleComponent* networkResimulationSingleComponent = nullptr;

    IClient* client = nullptr;
    FpsMeter fpsMeter;
    uint32 realCurrFrameId = 0;
};
}
