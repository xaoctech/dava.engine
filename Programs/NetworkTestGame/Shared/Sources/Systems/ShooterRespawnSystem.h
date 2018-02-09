#pragma once

#include <Entity/SceneSystem.h>

#include <memory>

namespace DAVA
{
class Scene;
}

class HealthComponent;

class ShooterRespawnSystem final : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterRespawnSystem, DAVA::SceneSystem);

    ShooterRespawnSystem(DAVA::Scene* scene);
    void ProcessFixed(DAVA::float32 dt) override;
    void PrepareForRemove() override;

private:
    DAVA::ComponentGroup<HealthComponent>* healthComponents;
    std::unique_ptr<DAVA::ComponentGroupOnAdd<HealthComponent>> healthComponentsPending;
};