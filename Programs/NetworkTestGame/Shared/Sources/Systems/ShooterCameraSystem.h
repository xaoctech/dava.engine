#pragma once

#include <Entity/SceneSystem.h>

namespace DAVA
{
class Scene;
class CameraComponent;
}

class ShooterAimComponent;

// System that aligns camera with the aim (client only)
class ShooterCameraSystem final : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterCameraSystem, DAVA::SceneSystem);

    ShooterCameraSystem(DAVA::Scene* scene);
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void Process(DAVA::float32 dt) override;
    void PrepareForRemove() override;

private:
    DAVA::CameraComponent* const cameraComponent;
    ShooterAimComponent* trackedAimComponent;
};