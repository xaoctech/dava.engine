#pragma once

#include <Entity/SceneSystem.h>

namespace DAVA
{
class Scene;
class UDPClient;

class NetworkHealthCheckSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkHealthCheckSystem, SceneSystem);

    NetworkHealthCheckSystem(Scene* scene);
    void Connect(const DAVA::String& host, uint16 port);

    void Process(float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    std::unique_ptr<UDPClient> client;
    float32 heartbeatTimeout = 0.f;
    const float32 maxHeartbeatTimeout = 1.f;
};
}
