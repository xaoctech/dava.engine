#pragma once

#include "Components/ShooterMirroredCharacterComponent.h"

#include <Entity/SceneSystem.h>

namespace DAVA
{
class Scene;
}

// Syncs CCT and its mirror transforms
class ShooterMirroredCharacterSystem final : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterMirroredCharacterSystem, DAVA::SceneSystem);

    ShooterMirroredCharacterSystem(DAVA::Scene* scene);
    void ProcessFixed(DAVA::float32 dt) override;
    void PrepareForRemove() override;

private:
    DAVA::ComponentGroup<ShooterMirroredCharacterComponent>* ccts;
};