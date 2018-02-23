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
}

class GameModeSystem : public DAVA::SceneSystem
{
public:
    enum GameModeType
    {
        WAITING,
        BATTLE
    };

    DAVA_VIRTUAL_REFLECTION(GameModeSystem, DAVA::SceneSystem);

    GameModeSystem(DAVA::Scene* scene);
    void Process(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};
    void OnClientConnected(const DAVA::Responder& responder);

private:
    void ProcessWaitingGameMode(DAVA::NetworkGameModeSingleComponent* netGameModeComponent);
    void ProcessBattleGameMode(DAVA::NetworkGameModeSingleComponent* netGameModeComponent);
    void TuneComponentPrivacy();

    DAVA::Camera* camera = nullptr;
    DAVA::IServer* server;
    DAVA::Vector<const DAVA::Responder*> connectedResponders;
};
