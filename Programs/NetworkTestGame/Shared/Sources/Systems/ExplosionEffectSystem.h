#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class EntityGroup;
class Scene;
}

class ExplosionEffectComponent;

class ExplosionEffectSystem : public DAVA::SceneSystem
{
    DAVA_VIRTUAL_REFLECTION(ExplosionEffectSystem, DAVA::SceneSystem);

public:
    ExplosionEffectSystem(DAVA::Scene* scene);
    ~ExplosionEffectSystem() override;

    void ProcessFixed(DAVA::float32 timeElapsed) override;

private:
    DAVA::Entity* CreateExplosionEffect(int type);

    DAVA::EntityGroup* rocketEntities = nullptr;
    DAVA::ComponentGroup<ExplosionEffectComponent>* explosionEffectComponents = nullptr;

    DAVA::Entity* explosionModel1 = nullptr;
    DAVA::Entity* explosionModel2 = nullptr;
};
