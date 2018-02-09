#pragma once

#include "Entity/SceneSystem.h"
#include "Base/FastName.h"
#include "Base/BaseTypes.h"
#include <NetworkCore/NetworkTypes.h>

namespace DAVA
{
class Scene;
class Entity;
class TransformComponent;
}

class GameVisibilitySystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(GameVisibilitySystem, DAVA::SceneSystem);

    GameVisibilitySystem(DAVA::Scene* scene);

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void PrepareForRemove() override{};
    void ProcessFixed(DAVA::float32 timeElapsed) override;

    void SetMaxAOI(DAVA::float32 maxAOI);
    DAVA::float32 GetMaxAOI() const;
    void SetPeriodIncreaseDistance(DAVA::float32 periodIncreaseDistance);
    DAVA::float32 GetPeriodIncreaseDistance() const;

private:
    struct CacheItem
    {
        DAVA::Entity* entity;
        DAVA::NetworkPlayerID playerID;
        DAVA::TransformComponent* transformComp;
    };

    DAVA::Vector<CacheItem> entities;
    DAVA::Vector<CacheItem> playerEntities;

    DAVA::float32 maxAOI;
    DAVA::float32 periodIncreaseDistance;

    void ObserveEntity(const CacheItem& item) const;
};
