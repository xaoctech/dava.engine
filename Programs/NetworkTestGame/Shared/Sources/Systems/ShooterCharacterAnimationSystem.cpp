#include "Systems/ShooterCharacterAnimationSystem.h"
#include "Components/ShooterRoleComponent.h"
#include "Components/ShooterAimComponent.h"
#include "ShooterConstants.h"
#include "ShooterUtils.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/MotionComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/SingleComponents/MotionSingleComponent.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>
#include <Entity/ComponentUtils.h>
#include <Debug/ProfilerCPU.h>

#include <NetworkCore/NetworkCoreUtils.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterCharacterAnimationSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterCharacterAnimationSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &ShooterCharacterAnimationSystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::NORMAL, 6.0f)]
    .End();
}

ShooterCharacterAnimationSystem::ShooterCharacterAnimationSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene, DAVA::ComponentUtils::MakeMask<ShooterRoleComponent>())
{
}

void ShooterCharacterAnimationSystem::AddEntity(DAVA::Entity* entity)
{
    ShooterRoleComponent* roleComponent = entity->GetComponent<ShooterRoleComponent>();
    DVASSERT(roleComponent != nullptr);

    if (roleComponent->GetRole() == ShooterRoleComponent::Role::Player)
    {
        data d;
        animationData.insert({ entity, d });
    }
}

void ShooterCharacterAnimationSystem::RemoveEntity(DAVA::Entity* entity)
{
    animationData.erase(entity);
}

void ShooterCharacterAnimationSystem::Process(DAVA::float32 dt)
{
    using namespace DAVA;

    DAVA_PROFILER_CPU_SCOPE("ShooterCarSystem::Process");

    for (auto& animData : animationData)
    {
        Entity* entity = animData.first;
        data& d = animData.second;

        if (!d.filled)
        {
            if (SetCharacterEntity(entity, d))
            {
                d.filled = true;
            }
        }

        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), entity);
        if (!allActions.empty())
        {
            const auto& actions = allActions.back();

            Vector2 direction(0.0f, 0.0f);
            bool accelerated = false;
            for (const FastName& action : actions.digitalActions)
            {
                if (action == SHOOTER_ACTION_MOVE_FORWARD)
                    direction.y += 1.f;
                else if (action == SHOOTER_ACTION_MOVE_BACKWARD)
                    direction.y -= 1.f;
                else if (action == SHOOTER_ACTION_MOVE_LEFT)
                    direction.x -= 1.f;
                else if (action == SHOOTER_ACTION_MOVE_RIGHT)
                    direction.x += 1.f;
                else if (action == SHOOTER_ACTION_ACCELERATE)
                    accelerated = true;
            }

            for (const auto& analogActionInfo : actions.analogActions)
            {
                if (analogActionInfo.first.actionId == SHOOTER_ACTION_ANALOG_MOVE)
                {
                    Vector2 analogActionState = ConvertFixedPrecisionToAnalog(analogActionInfo.first.precision, analogActionInfo.second);

                    if (!FLOAT_EQUAL_EPS(analogActionState.x, 0.0f, 0.1f))
                    {
                        direction.x = analogActionState.x < 0.0f ? -1.0f : 1.0f;
                    }

                    if (!FLOAT_EQUAL_EPS(analogActionState.y, 0.0f, 0.1f))
                    {
                        direction.y = -analogActionState.y < 0.0f ? -1.0f : 1.0f;
                    }

                    if (std::abs(analogActionState.x) > 0.9f || std::abs(analogActionState.y) > 0.9f)
                    {
                        // Accelerate
                        accelerated = true;
                    }
                }
            }

            ProcessActions(direction, accelerated, actions.digitalActions, dt, entity, d);
        }
    }
}

void ShooterCharacterAnimationSystem::PrepareForRemove()
{
}

// TODO: taken from TestCharacterController module with some changes, refactor it

bool ShooterCharacterAnimationSystem::SetCharacterEntity(DAVA::Entity* entity, data& d)
{
    using namespace DAVA;

    d.characterMeshEntity = nullptr;
    d.weaponEntity = nullptr;
    d.shootEffect = nullptr;

    d.characterMotionComponent = nullptr;
    d.characterSkeleton = nullptr;

    d.headJointIndex = DAVA::SkeletonComponent::INVALID_JOINT_INDEX;
    d.weaponPointJointIndex = DAVA::SkeletonComponent::INVALID_JOINT_INDEX;

    if (entity != nullptr)
    {
        d.characterEntity = SafeRetain(entity);
        d.characterMeshEntity = entity->FindByName("Character");
        if (d.characterMeshEntity == nullptr)
        {
            return false;
        }

        d.weaponEntity = entity->FindByName("Weapon");
        if (d.weaponEntity != nullptr)
            d.shootEffect = d.weaponEntity->FindByName("shot_auto");

        d.characterSkeleton = GetSkeletonComponent(d.characterMeshEntity);
        if (d.characterSkeleton == nullptr)
        {
            return false;
        }

        d.headJointIndex = d.characterSkeleton->GetJointIndex(FastName("node-Head"));

        d.weaponPointJointIndex = d.characterSkeleton->GetJointIndex(FastName("node-RH_WP"));
        if (d.weaponPointJointIndex == SkeletonComponent::INVALID_JOINT_INDEX)
            d.weaponPointJointIndex = d.characterSkeleton->GetJointIndex(FastName("node-Weapon_Primary"));

        DVASSERT(d.headJointIndex != SkeletonComponent::INVALID_JOINT_INDEX);
        DVASSERT(d.weaponPointJointIndex != SkeletonComponent::INVALID_JOINT_INDEX);

        d.characterMotionComponent = GetMotionComponent(d.characterMeshEntity);
    }

    return true;
}

void ShooterCharacterAnimationSystem::ProcessActions(DAVA::Vector2 moveDirectionTarget, bool running, const DAVA::Vector<DAVA::FastName>& actions, DAVA::float32 dt, DAVA::Entity* entity, data& d)
{
    using namespace DAVA;

    if (d.characterMotionComponent == nullptr)
        return;

    dt *= d.characterMotionComponent->GetPlaybackRate();

    //////////////////////////////////////////////////////////////////////////
    //Calculate motion params

    bool shooting = false;
    for (const FastName& action : actions)
    {
        if (action == SHOOTER_ACTION_ATTACK_BULLET)
            shooting = true;
    }

    float32 directionDt = dt * 5.f;

    if (d.directionParam.x > moveDirectionTarget.x)
        d.directionParam.x -= directionDt;
    if (d.directionParam.x < moveDirectionTarget.x)
        d.directionParam.x += directionDt;

    if (d.directionParam.y > moveDirectionTarget.y)
        d.directionParam.y -= directionDt;
    if (d.directionParam.y < moveDirectionTarget.y)
        d.directionParam.y += directionDt;

    if (Abs(d.directionParam.x) < directionDt)
        d.directionParam.x = 0.f;
    if (Abs(d.directionParam.y) < directionDt)
        d.directionParam.y = 0.f;

    d.directionParam.x = Clamp(d.directionParam.x, -1.f, 1.f);
    d.directionParam.y = Clamp(d.directionParam.y, -1.f, 1.f);

    d.isMoving = (d.directionParam.SquareLength() > EPSILON || !moveDirectionTarget.IsZero());
    d.isCrouching = false; //(keyboard != nullptr) && keyboard->GetKeyState(eInputElements::KB_LCTRL).IsPressed();
    d.isRun = d.isMoving && running && !shooting; //isMoving && !isCrouching && !waitReloadEnd && (keyboard != nullptr) && keyboard->GetKeyState(eInputElements::KB_LSHIFT).IsPressed();
    d.isZooming = false; //!isRun && ((mouse != nullptr && mouse->GetRightButtonState().IsPressed()) || doubleTapped);

    d.runningParam += (d.isRun ? dt : -dt) * 3.f;
    d.runningParam = Clamp(d.runningParam, 0.f, 1.f);

    d.crouchingParam += (d.isCrouching ? dt : -dt) * 3.f;
    d.crouchingParam = Clamp(d.crouchingParam, 0.f, 1.f);

    d.zoomFactor += (d.isZooming ? dt : -dt) * 3.f;
    d.zoomFactor = Clamp(d.zoomFactor, 0.f, 1.f);

    ShooterAimComponent* aimComponent = d.characterEntity->GetComponent<ShooterAimComponent>();
    if (aimComponent != nullptr)
    {
        Vector3 defaultAimRayEnd = SHOOTER_AIM_OFFSET + SHOOTER_MAX_SHOOTING_DISTANCE * SHOOTER_CHARACTER_FORWARD;
        defaultAimRayEnd.Normalize();
        float32 defaultAngle = std::acos(SHOOTER_CHARACTER_FORWARD.DotProduct(defaultAimRayEnd));

        d.aimAngleParam = RadToDeg(-aimComponent->GetCurrentAngleX() + defaultAngle);
    }
    else
    {
        d.aimAngleParam = 0.0f;
    }
    //////////////////////////////////////////////////////////////////////////

    const static FastName WEAPON_MOTION_NAME("WeaponMotion");
    const static FastName WEAPON_MOTION_RELOAD_STATE_ID("reload");
    const static FastName WEAPON_MOTION_SHOOT_STATE_ID("shoot");
    const static FastName WEAPON_MOTION_SHOOT_MARKER("shoot");

    MotionSingleComponent* msc = GetScene()->GetSingletonComponent<MotionSingleComponent>();
    if (d.waitReloadEnd)
    {
        MotionSingleComponent::AnimationInfo reloadAnimationInfo(d.characterMotionComponent, WEAPON_MOTION_NAME, WEAPON_MOTION_RELOAD_STATE_ID);
        d.waitReloadEnd = (msc->animationEnd.count(reloadAnimationInfo) == 0);
    }

    if (d.shootEffect != nullptr && shooting)
    {
        GetParticleEffectComponent(d.shootEffect)->Start();
    }

    //////////////////////////////////////////////////////////////////////////
    //Setup motion animation

    const static FastName MOTION_PARAM_RUNNING("running");
    const static FastName MOTION_PARAM_CROUCHING("crouching");
    const static FastName MOTION_PARAM_AIM_ANGLE("aim-angle");
    const static FastName MOTION_PARAM_DIRECTION_X("direction-x");
    const static FastName MOTION_PARAM_DIRECTION_Y("direction-y");

    const static FastName TRIGGER_MOVE("move");
    const static FastName TRIGGER_STOP("stop");
    const static FastName TRIGGER_WEAPON_SHOOT("shoot");
    const static FastName TRIGGER_WEAPON_RELOAD("reload");
    const static FastName TRIGGER_WEAPON_IDLE("idle");

    d.characterMotionComponent->SetParameter(MOTION_PARAM_RUNNING, d.runningParam);
    d.characterMotionComponent->SetParameter(MOTION_PARAM_CROUCHING, d.crouchingParam);
    d.characterMotionComponent->SetParameter(MOTION_PARAM_AIM_ANGLE, d.aimAngleParam);
    d.characterMotionComponent->SetParameter(MOTION_PARAM_DIRECTION_X, d.directionParam.x);
    d.characterMotionComponent->SetParameter(MOTION_PARAM_DIRECTION_Y, d.directionParam.y);

    if (!d.isRun)
    {
        if (shooting)
        {
            d.characterMotionComponent->TriggerEvent(TRIGGER_WEAPON_SHOOT);
        }
        else
        {
            d.characterMotionComponent->TriggerEvent(TRIGGER_WEAPON_IDLE);
        }
    }
    else
    {
        d.characterMotionComponent->TriggerEvent(TRIGGER_WEAPON_IDLE);
    }

    if (d.isMoving)
    {
        d.characterMotionComponent->TriggerEvent(TRIGGER_MOVE);
    }
    else
    {
        d.characterMotionComponent->TriggerEvent(TRIGGER_STOP);
    }

    // Weapon

    SkeletonComponent* skeleton = d.characterSkeleton;
    Vector3 weaponPointPosition = skeleton->GetJointObjectSpaceTransform(d.weaponPointJointIndex).GetPosition();
    Quaternion weaponPointOrientation = skeleton->GetJointObjectSpaceTransform(d.weaponPointJointIndex).GetOrientation();

    Matrix4 weaponTransform = Matrix4::MakeRotation(Vector3::UnitX, DegToRad(90.f));
    weaponTransform *= weaponPointOrientation.GetMatrix() * Matrix4::MakeTranslation(weaponPointPosition);

    d.weaponEntity->SetLocalTransform(weaponTransform);
}
