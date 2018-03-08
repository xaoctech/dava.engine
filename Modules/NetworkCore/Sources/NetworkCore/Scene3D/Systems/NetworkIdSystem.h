#pragma once

#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"

#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
class NetworkEntitiesSingleComponent;
class NetworkIdSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkIdSystem, SceneSystem);

    NetworkIdSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

private:
    bool isServer = false;
    uint32 playerOwnIdCounter = 0;

    NetworkEntitiesSingleComponent* networkEntities;
};
} // namespace DAVA
