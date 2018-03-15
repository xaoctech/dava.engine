#pragma once

#include "Entity/SceneSystem.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

namespace DAVA
{
class NetworkConnectionsSingleComponent;

class NetworkConnectionsSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkConnectionsSystem, SceneSystem);

    NetworkConnectionsSystem(Scene* scene);

    void PrepareForRemove() override{};

private:
    IServer* server;
    NetworkConnectionsSingleComponent* networkConnections;
};
}
