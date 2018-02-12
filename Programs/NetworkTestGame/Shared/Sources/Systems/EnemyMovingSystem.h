#pragma once

#include "Scene3D/Systems/BaseSimulationSystem.h"
#include "Base/BaseTypes.h"
#include "NetworkCore/NetworkTypes.h"

namespace DAVA
{
class Scene;
class Entity;
class NetworkReplicationSingleComponent;
}

class GameInputSystem;
class EnemyMovingSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(EnemyMovingSystem, DAVA::BaseSimulationSystem);

    EnemyMovingSystem(DAVA::Scene* scene);

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void Simulate(DAVA::Entity* entity) override;

private:
    GameInputSystem* gameInputSystem = nullptr;
    DAVA::NetworkReplicationSingleComponent* replicationSingleComponent = nullptr;
};
