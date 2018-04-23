#pragma once

#include "NetworkCore/UDPTransport/UDPServer.h"
#include <Scene3D/Systems/BaseSimulationSystem.h>

#include <Entity/SceneSystem.h>
#include <Utils/FpsMeter.h>

namespace DAVA
{
class NetworkServerConnectionsSingleComponent;
class IClient;
class NetworkTimeSingleComponent;
class NetworkResimulationSingleComponent;

class NetworkTimeSystem : public BaseSimulationSystem, IServerSyncCallback
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkTimeSystem, BaseSimulationSystem);

    NetworkTimeSystem(Scene* scene);

    void ReSimulationStart() override;
    void ReSimulationEnd() override;

    void OnConnectClient();
    void OnReceiveClient(const uint8* data, size_t, uint8 channelId, uint32);

    void ProcessFixedUpdateStats(float32 timeElapsed);
    void ProcessFixedSendStats(float32 timeElapsed);
    void Process(float32 timeElapsed) override;
    void PrepareForRemove() override{};
    void ProcessFrameDiff(int32 diff);

private:
    /// IServerSyncCallback
    void OnConnectServer(const Responder& responder) final;
    void OnReceiveServer(const Responder& responder, const void* data, size_t) final;
    /// end IServerSyncCallback

    NetworkTimeSingleComponent* netTimeComp = nullptr;
    NetworkResimulationSingleComponent* networkResimulationSingleComponent = nullptr;

    IServer* server = nullptr;
    IClient* client = nullptr;
    FpsMeter fpsMeter;
    FpsMeter ffpsMeter;
    uint32 realCurrFrameId = 0;
};
}
