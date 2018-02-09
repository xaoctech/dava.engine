#pragma once

#include "NetworkCore/UDPTransport/UDPServer.h"
#include <Scene3D/Systems/BaseSimulationSystem.h>

#include <Entity/SceneSystem.h>
#include <Utils/FpsMeter.h>

namespace DAVA
{
class IClient;

class NetworkTimeSystem : public SceneSystem, public ISimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkTimeSystem, SceneSystem);

    NetworkTimeSystem(Scene* scene);

    void SetSlowDownFactor(float32 value);
    void SetLossFactor(float32 value);

    const ComponentMask& GetResimulationComponents() const override;
    void ReSimulationStart(Entity* entity, uint32 frameId) override;
    void ReSimulationEnd(Entity* entity) override;
    void Simulate(Entity* entity) override;

    void OnConnectClient();
    void OnConnectServer(const Responder& responder);
    void OnReceiveClient(const uint8* data, size_t, uint8 channelId, uint32);
    void OnReceiveServer(const Responder& responder, const uint8* data, size_t);
    void ProcessFixed(float32 timeElapsed) override;
    void Process(float32 timeElapsed) override;
    void PrepareForRemove() override{};
    void ProcessFrameDiff(int32 diff);

private:
    IClient* client = nullptr;
    FpsMeter fpsMeter;
    uint32 realCurrFrameId = 0;
    float32 slowDownFactor = 0.01f;
    float32 lossFactor = 0.05f; // every 5% increase bucket size
};
}
