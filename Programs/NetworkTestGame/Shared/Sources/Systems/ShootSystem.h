#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/Systems/BaseSimulationSystem.h"

namespace DAVA
{
class Entity;
}
class ShootComponent;

class ShootSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShootSystem, DAVA::BaseSimulationSystem);

    ShootSystem(DAVA::Scene* scene);
    ~ShootSystem();

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void Simulate(DAVA::Entity* entity) override;

protected:
    mutable DAVA::Entity* bulletModel = nullptr;
    DAVA::Entity* GetBulletModel() const;

    void NextState(DAVA::Entity* bullet, ShootComponent* shootComponent, DAVA::float32 timeElapsed);
    DAVA::UnorderedSet<DAVA::Entity*> pendingEntities;
};
