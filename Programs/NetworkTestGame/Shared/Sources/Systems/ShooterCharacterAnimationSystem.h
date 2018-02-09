#pragma once

#include <Base/Vector.h>
#include <Base/Map.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Components/SkeletonComponent.h>

namespace DAVA
{
class Scene;
class MotionComponent;
}

class ShooterCharacterAnimationSystem final : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterCharacterAnimationSystem, DAVA::SceneSystem);

    ShooterCharacterAnimationSystem(DAVA::Scene* scene);
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void Process(DAVA::float32 dt) override;
    void PrepareForRemove() override;

private:
    // MOVED

    struct data
    {
        bool filled = false;

        DAVA::Entity* characterEntity = nullptr;

        DAVA::Entity* characterMeshEntity = nullptr;
        DAVA::Entity* weaponEntity = nullptr;
        DAVA::Entity* shootEffect = nullptr;
        DAVA::MotionComponent* characterMotionComponent = nullptr;
        DAVA::SkeletonComponent* characterSkeleton = nullptr;

        DAVA::Vector2 inputJoypadDirection;
        DAVA::Vector2 inputBeginPosition;
        DAVA::Vector2 inputEndPosition;

        DAVA::Vector3 characterForward;
        DAVA::Vector3 characterLeft;
        DAVA::Vector3 cameraDirection;
        DAVA::float32 cameraAngle = 0.f;

        DAVA::Vector2 directionParam;
        DAVA::float32 runningParam = 0.f;
        DAVA::float32 crouchingParam = 0.f;
        DAVA::float32 aimAngleParam = 0.f;
        DAVA::float32 zoomFactor = 0.f;

        DAVA::uint32 headJointIndex = DAVA::SkeletonComponent::INVALID_JOINT_INDEX;
        DAVA::uint32 weaponPointJointIndex = DAVA::SkeletonComponent::INVALID_JOINT_INDEX;

        bool isMoving = false;
        bool isRun = false;
        bool isCrouching = false;
        bool isZooming = false;

        bool doubleTapped = false;
        bool waitReloadEnd = false;
    };

    bool SetCharacterEntity(DAVA::Entity* entity, data& d);

    void ProcessActions(DAVA::Vector2 moveDirection, bool running, const DAVA::Vector<DAVA::FastName>& actions, DAVA::float32 dt, DAVA::Entity* entity, data& d);

private:
    DAVA::UnorderedMap<DAVA::Entity*, data> animationData;
};