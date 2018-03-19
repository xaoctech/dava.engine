#pragma once

#include "Entity/SceneSystem.h"
#include "Base/FastName.h"
#include "Base/Vector.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

namespace DAVA
{
class Scene;
class Entity;
class NetworkGameModeSingleComponent;
class Camera;
class NetworkServerConnectionsSingleComponent;
}

class InvaderConnectSystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(InvaderConnectSystem, DAVA::SceneSystem);

    explicit InvaderConnectSystem(DAVA::Scene* scene);
    void Process(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};
    void OnClientConnected(const DAVA::FastName& token);

private:
    DAVA::Camera* camera = nullptr;
    DAVA::NetworkServerConnectionsSingleComponent* netConnectionsComp = nullptr;
};
