#pragma once

#include "Components/PowerupComponent.h"

#include <limits>
#include <memory>

#include <Entity/SceneSystem.h>
#include <Entity/SingletonComponent.h>
#include <Reflection/Reflection.h>
#include <Base/ScopedPtr.h>

using namespace DAVA;

class PowerupSpawnSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(PowerupSpawnSystem, SceneSystem);

    PowerupSpawnSystem(Scene* scene);
    void PrepareForRemove() override;

    void ProcessFixed(float32 timeElapsed) override;

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

private:
    Entity* AddBonusModel(Entity* bonus);
    void TuneBonusModel(Entity* model, PowerupType bonusType);
    ScopedPtr<Entity> CreateBonusEntityOnServer();

    const uint32 bonusCountLimit = 5;
    uint32 bonusCount = 0;
    const float32 cooldown = 10.f;
    float32 lastPowerupTime = 0.f;
    bool isServer;

    ScopedPtr<Entity> refBonusModel;
    Vector<Entity*> pendingSetupEntities;
};
