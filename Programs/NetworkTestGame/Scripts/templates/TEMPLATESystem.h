#pragma once

#include "Scene3D/Systems/BaseSimulationSystem.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class Scene;
class Entity;
}

class TEMPLATESystem : public DAVA::BaseSimulationSystem
{
public:
    TEMPLATESystem(DAVA::Scene* scene);
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void Simulate(DAVA::Entity* entity) override;
    void ProcessFixed(DAVA::float32 timeElapsed) override;

private:
    DAVA::UnorderedSet<DAVA::Entity*> entities;
};
