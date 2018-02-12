#pragma once

#include "Scene3D/Systems/BaseSimulationSystem.h"
#include "Base/BaseTypes.h"
#include "Functional/Signal.h"

namespace DAVA
{
class Scene;
class Entity;
class Component;
}

class GameStunnableComponent;
class GameStunningComponent;

class GameStunningSystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(GameStunningSystem, DAVA::SceneSystem);

    GameStunningSystem(DAVA::Scene* scene);
    void PrepareForRemove() override{};

    void ProcessFixed(DAVA::float32 timeElapsed) override;

private:
    DAVA::ComponentGroup<GameStunnableComponent>* stunnableGroup = nullptr;
};
