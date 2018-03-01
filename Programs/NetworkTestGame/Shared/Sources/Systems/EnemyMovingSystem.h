#pragma once

#include <Base/BaseTypes.h>
#include <NetworkCore/NetworkTypes.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

namespace DAVA
{
class Scene;
}

class GameInputSystem;
class EnemyMovingSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(EnemyMovingSystem, DAVA::BaseSimulationSystem);

    EnemyMovingSystem(DAVA::Scene* scene);

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    GameInputSystem* gameInputSystem = nullptr;
    DAVA::EntityGroup* entityGroup = nullptr;
};
