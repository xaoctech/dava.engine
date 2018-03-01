#pragma once

#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"

namespace DAVA
{
class NetworkTransformFromLocalToNetSystem : public BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkTransformFromLocalToNetSystem, BaseSimulationSystem);

    NetworkTransformFromLocalToNetSystem(Scene* scene);

    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override;

    void ReSimulationStart() override;
    void ReSimulationEnd() override;

private:
    void CopyFromLocalToNet(Entity* entity);

    EntityGroup* entities = nullptr;
};
} // namespace DAVA
