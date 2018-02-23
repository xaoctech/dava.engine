#pragma once

#include "Components/ShooterMirroredCharacterComponent.h"

#include <Base/Set.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class Scene;
}

// Handles collisions between a player and a car
class ShooterCarAttackSystem final : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterCarAttackSystem, DAVA::SceneSystem);

    ShooterCarAttackSystem(DAVA::Scene* scene);
    void ProcessFixed(DAVA::float32 dt) override;
    void PrepareForRemove() override;

private:
    DAVA::ComponentGroup<ShooterMirroredCharacterComponent>* ccts;
    DAVA::UnorderedSet<ShooterMirroredCharacterComponent*> pushedCcts;
};