#pragma once

#include <Entity/SceneSystem.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>

namespace DAVA
{
class Scene;
}

class ShooterCharacterAnimationSystem final : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterCharacterAnimationSystem, DAVA::SceneSystem);

    ShooterCharacterAnimationSystem(DAVA::Scene* scene);
    void ProcessFixed(DAVA::float32 dt) override;
    void PrepareForRemove() override;

    void UpdateInputDependentParams(const DAVA::Entity* player, const DAVA::ActionsSingleComponent::Actions& actions, DAVA::float32 dt);
    void UpdateInputIndependentParams(const DAVA::Entity* player, const DAVA::float32 dt);
    void UpdateEventsAndWeapon(DAVA::Entity* player);

private:
    DAVA::EntityGroup* players;
};