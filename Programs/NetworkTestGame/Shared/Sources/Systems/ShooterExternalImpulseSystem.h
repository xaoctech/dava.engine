#pragma once

#include <Base/Set.h>
#include <Entity/SceneSystem.h>
#include <Math/Vector.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

namespace DAVA
{
class Scene;
class CapsuleCharacterControllerComponent;

// Applies external impulse to player
class ShooterExternalImpulseSystem final : public BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterExternalImpulseSystem, SceneSystem);

    explicit ShooterExternalImpulseSystem(Scene* scene);
    void ProcessFixed(float32 dt) override;
    void PrepareForRemove() override;

private:
    EntityGroup* entityGroup = nullptr;
};
}
