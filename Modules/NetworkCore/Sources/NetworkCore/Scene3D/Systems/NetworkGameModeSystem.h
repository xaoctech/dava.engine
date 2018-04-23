#pragma once

#include "NetworkCore/UDPTransport/UDPClient.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

#include <Entity/SceneSystem.h>

namespace DAVA
{
class Scene;
class ActionsSingleComponent;

class NetworkGameModeSingleComponent;
class NetworkServerConnectionsSingleComponent;

class NetworkGameModeSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkGameModeSystem, SceneSystem);

    NetworkGameModeSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override{};

    void ProcessFixed(float32 timeElapsed) override;

private:
    void OnTokenConfirmationServer(const Responder& responder);
    void OnDisconnectServer(const FastName& token);
    void OnReceiveServer(const Responder& responder, const uint8* data, size_t size);
    void OnReceiveClient(const uint8* data, size_t, uint8, uint32);

    IServer* server = nullptr;
    IClient* client = nullptr;

    NetworkGameModeSingleComponent* netGameModeComp = nullptr;
    NetworkServerConnectionsSingleComponent* netConnectionsComp = nullptr;
    ActionsSingleComponent* actionSingleComponent = nullptr;
};
}
