#pragma once

#include "Base/BaseMath.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class Camera;
class UIEvent;
class InputCallback;
class Motion;
class MotionComponent;
class SkeletonComponent;
class CharacterControllerComponent;
};

class CharacterMoveSystem;
class CharacterCameraSystem;
class CharacterControllerSystem : public DAVA::SceneSystem
{
public:
    CharacterControllerSystem(DAVA::Scene* scene);
    virtual ~CharacterControllerSystem();

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void PrepareForRemove() override;

    void Process(DAVA::float32 timeElapsed) override;
    bool Input(DAVA::UIEvent* uiEvent) override;

private:
    DAVA::Entity* characterEntity = nullptr;
    DAVA::Camera* camera = nullptr;
    DAVA::CharacterControllerComponent* controllerComponent = nullptr;

    DAVA::Entity* characterMeshEntity = nullptr;
    DAVA::MotionComponent* characterMotionComponent = nullptr;
    DAVA::Motion* moveMotion = nullptr;
    DAVA::Motion* aimMotion = nullptr;

    DAVA::Vector3 characterForward;
    DAVA::Vector3 characterLeft;

    DAVA::Vector2 directionParam;
    DAVA::float32 runningParam = 0.f;
    DAVA::float32 crouchingParam = 0.f;
    DAVA::float32 aimAngleParam = 0.f;
    DAVA::float32 zoomFactor = 0.f;

    DAVA::uint32 headJointIndex = 0;

    bool isMoving = false;
    bool isRun = false;
    bool isCrouching = false;
    bool isZooming = false;

    bool characterInited = false;

    friend class CharacterMoveSystem;
    friend class CharacterCameraSystem;
};

class CharacterMoveSystem : public DAVA::SceneSystem
{
public:
    CharacterMoveSystem(DAVA::Scene* scene);

    void PrepareForRemove() override;
    void Process(DAVA::float32 timeElapsed) override;

protected:
    CharacterControllerSystem* controllerSystem = nullptr;
};

class CharacterCameraSystem : public DAVA::SceneSystem
{
public:
    CharacterCameraSystem(DAVA::Scene* scene);

    void PrepareForRemove() override;
    void Process(DAVA::float32 timeElapsed) override;

protected:
    CharacterControllerSystem* controllerSystem = nullptr;
};
