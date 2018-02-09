#pragma once

#include "Entity/SceneSystem.h"
#include "Base/BaseTypes.h"

#include "Game.h"

namespace DAVA
{
class Scene;
class Entity;
class NetworkRemoteInputSystem;
}

class CreateGameModeSystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(CreateGameModeSystem, DAVA::SceneSystem);

    CreateGameModeSystem(DAVA::Scene* scene);

    void Process(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    void CreateGameSystems(GameMode::Id gameModeId);
    bool isInit = false;
    DAVA::NetworkRemoteInputSystem* remoteInputSystem = nullptr;
};
