#include "CharacterControllerSystem.h"

#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

#include "Render/Highlevel/Camera.h"

#include "Input/InputSystem.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"

#include "Utils/Utils.h"

#include "DeviceManager/DeviceManager.h"

#include "Physics/PhysicsUtils.h"
#include "Physics/CharacterControllerComponent.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/SkeletonAnimation/Motion.h"
#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

#include "Render/Highlevel/RenderSystem.h"
#include "Render/RenderHelper.h"

#include "Logger/Logger.h"

using namespace DAVA;

CharacterControllerSystem::CharacterControllerSystem(Scene* scene)
    : SceneSystem(scene)
{
    characterForward = Vector3::UnitX;

    characterLeft = Vector3::UnitZ.CrossProduct(characterForward);
}

CharacterControllerSystem::~CharacterControllerSystem()
{
    SafeRelease(characterEntity);
    SafeRelease(camera);
}

void CharacterControllerSystem::AddEntity(Entity* entity)
{
    DVASSERT(characterEntity == nullptr);

    DVASSERT(GetCamera(entity) != nullptr);
    DVASSERT(PhysicsUtils::GetCharacterControllerComponent(entity) != nullptr);

    characterEntity = SafeRetain(entity);
    characterMeshEntity = entity->FindByName("Character");
    DVASSERT(characterMeshEntity != nullptr);

    weaponEntity = entity->FindByName("Weapon");
    DVASSERT(weaponEntity != nullptr);

    shootEffect = weaponEntity->FindByName("shot_auto");
    DVASSERT(shootEffect != nullptr);

    camera = SafeRetain(GetCamera(entity));
    controllerComponent = PhysicsUtils::GetCharacterControllerComponent(entity);

    characterMotionComponent = GetMotionComponent(characterMeshEntity);
    //characterMotionComponent->SetPlaybackRate(0.1f);
}

void CharacterControllerSystem::RemoveEntity(Entity* entity)
{
    DVASSERT(characterEntity == entity);

    SafeRelease(characterEntity);
    characterMeshEntity = nullptr;
    weaponEntity = nullptr;
    shootEffect = nullptr;

    SafeRelease(camera);
    controllerComponent = nullptr;
    characterMotionComponent = nullptr;
    moveMotion = nullptr;
    aimMotion = nullptr;
    weaponMotion = nullptr;
    characterInited = false;
}

void CharacterControllerSystem::PrepareForRemove()
{
    if (characterEntity != nullptr)
        RemoveEntity(characterEntity);
}

void CharacterControllerSystem::Process(float32 timeElapsed)
{
    if (!GetScene()->motionSingleComponent->reloadConfig.empty() || characterMotionComponent == nullptr)
        return;

    timeElapsed *= characterMotionComponent->GetPlaybackRate();

    if (!characterInited)
    {
        uint32 motionCount = characterMotionComponent->GetMotionsCount();
        for (uint32 m = 0; m < motionCount; ++m)
        {
            Motion* motion = characterMotionComponent->GetMotion(m);
            const FastName& motionName = motion->GetName();
            if (motionName == FastName("MoveMotion"))
            {
                moveMotion = motion;
                moveMotion->BindParameter(FastName("running"), &runningParam);
                moveMotion->BindParameter(FastName("crouching"), &crouchingParam);
                moveMotion->BindParameter(FastName("direction-x"), &directionParam.x);
                moveMotion->BindParameter(FastName("direction-y"), &directionParam.y);
            }
            else if (motionName == FastName("AimMotion"))
            {
                aimMotion = motion;
                aimMotion->BindParameter(FastName("running"), &runningParam);
                aimMotion->BindParameter(FastName("aim-angle"), &aimAngleParam);
            }
            else if (motionName == FastName("WeaponMotion"))
            {
                weaponMotion = motion;
            }
        }

        SkeletonComponent* skeleton = GetSkeletonComponent(characterMeshEntity);
        DVASSERT(skeleton != nullptr);

        headJointIndex = skeleton->GetJointIndex(FastName("node-Head"));
        weaponPointJointIndex = skeleton->GetJointIndex(FastName("node-RH_WP"));

        DVASSERT(headJointIndex != SkeletonComponent::INVALID_JOINT_INDEX);
        DVASSERT(weaponPointJointIndex != SkeletonComponent::INVALID_JOINT_INDEX);

        characterInited = true;
    }

    characterLeft = Vector3::UnitZ.CrossProduct(characterForward);

    Quaternion characterOrientation;
    characterOrientation.Construct(-Vector3::UnitY, characterForward);
    characterMeshEntity->SetLocalTransform(characterOrientation.GetMatrix());

    //////////////////////////////////////////////////////////////////////////

    Keyboard* keyboard = GetEngineContext()->deviceManager->GetKeyboard();
    Mouse* mouse = GetEngineContext()->deviceManager->GetMouse();

    Vector3 moveDirectionTarget;
    if (keyboard->GetKeyState(eInputElements::KB_W).IsPressed() || keyboard->GetKeyState(eInputElements::KB_UP).IsPressed())
        moveDirectionTarget.y += 1.f;

    if (keyboard->GetKeyState(eInputElements::KB_S).IsPressed() || keyboard->GetKeyState(eInputElements::KB_DOWN).IsPressed())
        moveDirectionTarget.y -= 1.f;

    if (keyboard->GetKeyState(eInputElements::KB_A).IsPressed() || keyboard->GetKeyState(eInputElements::KB_LEFT).IsPressed())
        moveDirectionTarget.x -= 1.f;

    if (keyboard->GetKeyState(eInputElements::KB_D).IsPressed() || keyboard->GetKeyState(eInputElements::KB_RIGHT).IsPressed())
        moveDirectionTarget.x += 1.f;

    float32 directionDt = timeElapsed * 5.f;

    if (directionParam.x > moveDirectionTarget.x)
        directionParam.x -= directionDt;
    if (directionParam.x < moveDirectionTarget.x)
        directionParam.x += directionDt;

    if (directionParam.y > moveDirectionTarget.y)
        directionParam.y -= directionDt;
    if (directionParam.y < moveDirectionTarget.y)
        directionParam.y += directionDt;

    if (Abs(directionParam.x) < directionDt)
        directionParam.x = 0.f;
    if (Abs(directionParam.y) < directionDt)
        directionParam.y = 0.f;

    directionParam.x = Clamp(directionParam.x, -1.f, 1.f);
    directionParam.y = Clamp(directionParam.y, -1.f, 1.f);
    aimAngleParam = Clamp(aimAngleParam, -75.f, 75.f);

    isMoving = (directionParam.SquareLength() > EPSILON || !moveDirectionTarget.IsZero());
    isCrouching = keyboard->GetKeyState(eInputElements::KB_LCTRL).IsPressed();
    isRun = isMoving && !isCrouching && keyboard->GetKeyState(eInputElements::KB_LSHIFT).IsPressed();
    isZooming = !isRun && mouse->GetRightButtonState().IsPressed();

    //////////////////////////////////////////////////////////////////////////
    //Animation

    runningParam += (isRun ? timeElapsed : -timeElapsed) * 3.f;
    runningParam = Clamp(runningParam, 0.f, 1.f);

    crouchingParam += (isCrouching ? timeElapsed : -timeElapsed) * 3.f;
    crouchingParam = Clamp(crouchingParam, 0.f, 1.f);

    zoomFactor += (isZooming ? timeElapsed : -timeElapsed) * 3.f;
    zoomFactor = Clamp(zoomFactor, 0.f, 1.f);

    MotionSingleComponent* msc = GetScene()->motionSingleComponent;

    const static FastName TRIGGER_MOVE("move");
    const static FastName TRIGGER_STOP("stop");

    const static FastName TRIGGER_WEAPON_SHOOT("shoot");
    const static FastName TRIGGER_WEAPON_RELOAD("reload");
    const static FastName TRIGGER_WEAPON_IDLE("idle");

    if (!isRun)
    {
        if (keyboard->GetKeyState(eInputElements::KB_R).IsJustPressed())
        {
            weaponMotion->TriggerEvent(TRIGGER_WEAPON_RELOAD);
        }
        else
        {
            if (mouse->GetLeftButtonState().IsPressed())
            {
                weaponMotion->TriggerEvent(TRIGGER_WEAPON_SHOOT);

                shootingDelay -= timeElapsed;
                if (shootingDelay <= 0.0f)
                {
                    shootingDelay = 0.1f;
                    GetParticleEffectComponent(shootEffect)->Start();
                }
            }
            else
            {
                shootingDelay = 0.f;
                weaponMotion->TriggerEvent(TRIGGER_WEAPON_IDLE);
            }
        }
    }

    if (isMoving)
    {
        moveMotion->TriggerEvent(TRIGGER_MOVE);
    }
    else
    {
        moveMotion->TriggerEvent(TRIGGER_STOP);
    }
}

bool CharacterControllerSystem::Input(UIEvent* uiEvent)
{
    const static float32 MOUSE_SENSITIVITY = 2.f;
    if (uiEvent->device == eInputDevices::MOUSE && (uiEvent->phase == UIEvent::Phase::MOVE || uiEvent->phase == UIEvent::Phase::DRAG))
    {
        Size2f wndSize = uiEvent->window->GetSize();
        float32 relativeX = uiEvent->point.x / wndSize.dx;
        float32 relativeY = uiEvent->point.y / wndSize.dy;

        characterForward = Quaternion::MakeRotationFastZ(-relativeX * MOUSE_SENSITIVITY).ApplyToVectorFast(characterForward);
        characterForward.Normalize();
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////

CharacterMoveSystem::CharacterMoveSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
    controllerSystem = scene->GetSystem<CharacterControllerSystem>();
}

void CharacterMoveSystem::PrepareForRemove()
{
    controllerSystem = nullptr;
}

void CharacterMoveSystem::Process(DAVA::float32 timeElapsed)
{
    if (!controllerSystem->characterInited)
        return;

    Vector3 moveDisplacement = -controllerSystem->characterForward * controllerSystem->characterMotionComponent->rootOffsetDelta.y
    + controllerSystem->characterLeft * controllerSystem->characterMotionComponent->rootOffsetDelta.x;

    controllerSystem->controllerComponent->Move(moveDisplacement);
}

//////////////////////////////////////////////////////////////////////////

CharacterWeaponSystem::CharacterWeaponSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
    controllerSystem = scene->GetSystem<CharacterControllerSystem>();
}

void CharacterWeaponSystem::PrepareForRemove()
{
    controllerSystem = nullptr;
}

void CharacterWeaponSystem::Process(DAVA::float32 timeElapsed)
{
    if (!controllerSystem->characterInited)
        return;

    SkeletonComponent* skeleton = GetSkeletonComponent(controllerSystem->characterMeshEntity);
    Vector3 weaponPointPosition = skeleton->GetJointObjectSpaceTransform(controllerSystem->weaponPointJointIndex).GetPosition();
    Quaternion weaponPointOrientation = skeleton->GetJointObjectSpaceTransform(controllerSystem->weaponPointJointIndex).GetOrientation();

    Matrix4 weaponTransform = Matrix4::MakeRotation(Vector3::UnitX, DegToRad(90.f));
    weaponTransform *= weaponPointOrientation.GetMatrix() * Matrix4::MakeTranslation(weaponPointPosition);

    controllerSystem->weaponEntity->SetLocalTransform(weaponTransform);
}

//////////////////////////////////////////////////////////////////////////

CharacterCameraSystem::CharacterCameraSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
    controllerSystem = scene->GetSystem<CharacterControllerSystem>();
}

void CharacterCameraSystem::PrepareForRemove()
{
    controllerSystem = nullptr;
}

void CharacterCameraSystem::Process(DAVA::float32 timeElapsed)
{
    if (!controllerSystem->characterInited)
        return;

    Vector3 characterPosition = controllerSystem->characterEntity->GetLocalTransform().GetTranslationVector();

    Vector3 normalCameraOffset = -3.f * controllerSystem->characterForward + Vector3(0.f, 0.f, 1.9f);
    Vector3 normalTargetOffset = Vector3(0.f, 0.f, 1.4f);

    float32 headHeight = GetSkeletonComponent(controllerSystem->characterMeshEntity)->GetJointObjectSpaceTransform(controllerSystem->headJointIndex).GetPosition().z;
    Vector3 zoomCameraOffset = -1.4f * controllerSystem->characterForward - 0.4f * controllerSystem->characterLeft + Vector3(0.f, 0.f, headHeight + 0.15f);
    Vector3 zoomTargetOffset = -0.3f * controllerSystem->characterLeft + Vector3(0.f, 0.f, headHeight);

    Vector3 cameraOffset = Lerp(normalCameraOffset, zoomCameraOffset, controllerSystem->zoomFactor);
    Vector3 targetOffset = Lerp(normalTargetOffset, zoomTargetOffset, controllerSystem->zoomFactor);
    float32 fov = Lerp(70.f, 55.f, controllerSystem->zoomFactor);

    controllerSystem->camera->SetPosition(characterPosition + cameraOffset);
    controllerSystem->camera->SetTarget(characterPosition + targetOffset);
    controllerSystem->camera->SetFOV(fov);
}