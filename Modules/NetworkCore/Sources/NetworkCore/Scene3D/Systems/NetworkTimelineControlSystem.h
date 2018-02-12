#pragma once

#include <Entity/SceneSystem.h>
#include "NetworkCore/NetworkTypes.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

namespace DAVA
{
class Scene;
class IServer;
class IClient;
class NetworkTimelineSingleComponent;

class NetworkTimelineControlSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkTimelineControlSystem, SceneSystem);

    NetworkTimelineControlSystem(Scene* scene);
    void Process(float32 timeElapsed) override;
    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void OnConnectServer(const Responder& responder);
    void OnReceiveServer(const Responder& responder, const uint8* data, size_t);
    void OnReceiveClient(const uint8* data, size_t, uint8, uint32);

private:
    IServer* server = nullptr;
    IClient* client = nullptr;
    NetworkTimelineSingleComponent* netTimelineComp = nullptr;
    ServicePacketHeader packetHeader;

    void ProcessClient(NetworkTimelineSingleComponent* netTimelineComp);
    void ProcessServer(NetworkTimelineSingleComponent* netTimelineComp);

    void StepOver(NetworkTimelineSingleComponent* netTimelineComp) const;
    void SwitchPauseScene() const;
    void OnReceive(const uint8* data) const;
};
}
