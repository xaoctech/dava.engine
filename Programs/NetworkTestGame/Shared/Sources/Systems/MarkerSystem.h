#pragma once

#include "Scene3D/Systems/BaseSimulationSystem.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class Scene;
class Entity;
}

class MarkerSystem : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(MarkerSystem, DAVA::BaseSimulationSystem);

    MarkerSystem(DAVA::Scene* scene);
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void Simulate(DAVA::Entity* entity) override;
    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

    // TODO: move it to component\utils fns
    void SetHealthParams(DAVA::uint32 maxHealth, DAVA::float32 maxHealthBarHeight);

private:
    DAVA::Set<DAVA::Entity*> pendingEntities;
    DAVA::UnorderedMap<DAVA::Entity*, DAVA::Entity*> tankToBar;

    DAVA::uint32 maxHealth = 10;
    DAVA::float32 maxHealthBarHeight = 0.5f;
};
