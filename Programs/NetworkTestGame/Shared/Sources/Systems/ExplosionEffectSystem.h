#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class EntityGroup;
class NetworkEntitiesSingleComponent;
class Scene;
}

class ExplosionEffectComponent;
class EffectQueueSingleComponent;

class ExplosionEffectSystem : public DAVA::SceneSystem
{
    DAVA_VIRTUAL_REFLECTION(ExplosionEffectSystem, DAVA::SceneSystem);

public:
    ExplosionEffectSystem(DAVA::Scene* scene);
    ~ExplosionEffectSystem() override;

    void ProcessFixed(DAVA::float32 timeElapsed) override;

private:
    void ProcessNewEffects(DAVA::Scene* scene);
    DAVA::Entity* CreateExplosionEffect(int type);
    void LoadEffectModels();

    bool isGuiMode = true;

    DAVA::NetworkEntitiesSingleComponent* networkEntities = nullptr;
    const EffectQueueSingleComponent* effectQueue = nullptr;

    DAVA::ComponentGroup<ExplosionEffectComponent>* explosionEffectComponents = nullptr;

    DAVA::Vector<DAVA::Entity*> effectModelCache;
};
