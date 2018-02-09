#pragma once

#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"

namespace DAVA
{
class NetworkTransformFromLocalToNetSystem : public SceneSystem, public ISimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkTransformFromLocalToNetSystem, SceneSystem);

    NetworkTransformFromLocalToNetSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override;

    void ReSimulationStart(Entity* entity, uint32 frameId) override;
    void ReSimulationEnd(Entity* entity) override;
    void Simulate(Entity* entity) override;
    const ComponentMask& GetResimulationComponents() const override;

private:
    void CopyFromLocalToNet(Entity* entity);

    UnorderedSet<Entity*> entities;
};
} // namespace DAVA
