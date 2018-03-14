#pragma once

#include <Base/Set.h>
#include <Entity/SceneSystem.h>
#include <Math/Vector.h>

namespace DAVA
{
class Scene;
class CapsuleCharacterControllerComponent;
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
    DAVA::ComponentGroup<DAVA::CapsuleCharacterControllerComponent>* ccts;
    DAVA::UnorderedMap<DAVA::CapsuleCharacterControllerComponent*, DAVA::Vector3> pushedCcts;
};