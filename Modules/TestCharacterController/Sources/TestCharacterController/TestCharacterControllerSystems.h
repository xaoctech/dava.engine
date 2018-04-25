#pragma once

#include "Base/BaseMath.h"
#include "Entity/SceneSystem.h"

#include "Scene3D/Components/SkeletonComponent.h"

namespace DAVA
{
class MotionComponent;
class CharacterControllerComponent;

class TestCharacterMoveSystem;
class TestCharacterWeaponSystem;
class TestCharacterCameraSystem;

class TestCharacterControllerSystem : public SceneSystem
{
    DAVA_VIRTUAL_REFLECTION(TestCharacterControllerSystem, SceneSystem);

public:
    TestCharacterControllerSystem(Scene* scene);
    virtual ~TestCharacterControllerSystem();

    void PrepareForRemove() override;

    void Process(float32 timeElapsed) override;
    bool Input(UIEvent* uiEvent) override;

    void SetCharacterEntity(Entity* entity);
    void SetJoypadDirection(const Vector2& direction);

private:
    Entity* characterEntity = nullptr;
    CharacterControllerComponent* controllerComponent = nullptr;

    Entity* characterMeshEntity = nullptr;
    Entity* weaponEntity = nullptr;
    Entity* shootEffect = nullptr;
    MotionComponent* characterMotionComponent = nullptr;
    SkeletonComponent* characterSkeleton = nullptr;

    Vector2 inputJoypadDirection;
    Vector2 inputBeginPosition;
    Vector2 inputEndPosition;

    Vector3 characterForward;
    Vector3 characterLeft;
    Vector3 cameraDirection;
    float32 cameraAngle = 0.f;

    Vector2 directionParam;
    float32 runningParam = 0.f;
    float32 crouchingParam = 0.f;
    float32 aimAngleParam = 0.f;
    float32 zoomFactor = 0.f;

    uint32 headJointIndex = SkeletonComponent::INVALID_JOINT_INDEX;
    uint32 weaponPointJointIndex = SkeletonComponent::INVALID_JOINT_INDEX;

    bool isMoving = false;
    bool isRun = false;
    bool isCrouching = false;
    bool isZooming = false;

    bool doubleTapped = false;
    bool waitReloadEnd = false;

    friend class TestCharacterMoveSystem;
    friend class TestCharacterWeaponSystem;
    friend class TestCharacterCameraSystem;
};

class TestCharacterMoveSystem : public SceneSystem
{
    DAVA_VIRTUAL_REFLECTION(TestCharacterMoveSystem, SceneSystem);

public:
    TestCharacterMoveSystem(Scene* scene);

    void PrepareForRemove() override;
    void ProcessFixed(float32 timeElapsed) override;

protected:
    TestCharacterControllerSystem* controllerSystem = nullptr;
};

class TestCharacterWeaponSystem : public SceneSystem
{
    DAVA_VIRTUAL_REFLECTION(TestCharacterWeaponSystem, SceneSystem);

public:
    TestCharacterWeaponSystem(Scene* scene);

    void PrepareForRemove() override;
    void Process(float32 timeElapsed) override;

protected:
    TestCharacterControllerSystem* controllerSystem = nullptr;
};

class TestCharacterCameraSystem : public SceneSystem
{
    DAVA_VIRTUAL_REFLECTION(TestCharacterCameraSystem, SceneSystem);

public:
    TestCharacterCameraSystem(Scene* scene);

    void PrepareForRemove() override;
    void Process(float32 timeElapsed) override;

protected:
    TestCharacterControllerSystem* controllerSystem = nullptr;
};

} // namespace DAVA