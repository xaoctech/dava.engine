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

    NetworkIdSystem(Scene* scene_);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    static bool IsGeneratedFromAction(NetworkID entityId);
    static NetworkID GetEntityIdFromAction(FrameActionID frameActionId);
    static NetworkID GetEntityIdForStaticObject();
    static bool IsEntityIdForStaticObject(NetworkID networkID);

private:
    bool IsPredicted(Entity* entity, const Type* componentType);
    NetworkID GenerateUniqueId(Entity* entity);

    NetworkEntitiesSingleComponent* networkEntities;
};
} // namespace DAVA
