#pragma once

#pragma once

#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"

namespace DAVA
{
class NetworkTransformFromNetToLocalSystem : public BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkTransformFromNetToLocalSystem, BaseSimulationSystem);

    NetworkTransformFromNetToLocalSystem(Scene* scene);

    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override;

private:
    void CopyFromNetToLocal(Entity* entity);

    EntityGroup* entities = nullptr;
};
} // namespace DAVA
