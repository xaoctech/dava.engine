#include "Systems/ShooterCharacterAnimationSystem.h"
#include "Components/ShooterAimComponent.h"
#include "ShooterConstants.h"
#include "ShooterUtils.h"

#include <Debug/ProfilerCPU.h>
#include <Entity/ComponentUtils.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/MotionComponent.h>
#include <Scene3D/Components/SkeletonComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/SingleComponents/MotionSingleComponent.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>

#include <NetworkCore/NetworkCoreUtils.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterCharacterAnimationSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterCharacterAnimationSystem>::Begin()[M::SystemTags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterCharacterAnimationSystem::ProcessFixed)[M::SystemProcessInfo(SPI::Group::Gameplay, SPI::Type::Fixed, 25.0f)]
    .End();
}

ShooterCharacterAnimationSystem::ShooterCharacterAnimationSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene, DAVA::ComponentUtils::MakeMask())
{
    using namespace DAVA;

    players = scene->AquireEntityGroup<MotionComponent, SkeletonComponent, ShooterAimComponent>();
}

void ShooterCharacterAnimationSystem::ProcessFixed(DAVA::float32 dt)
{
    using namespace DAVA;

    for (Entity* player : players->GetEntities())
    {
        if (IsServer(this) || IsClientOwner(this, player))
        {
            const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), player);
            if (!allActions.empty())
            {
                for (const ActionsSingleComponent::Actions& actions : allActions)
                {
                    UpdateInputDependentParams(player, actions, dt);
                }
            }
        }

        UpdateInputIndependentParams(player, dt);
        UpdateEventsAndWeapon(player);
    }
}

void ShooterCharacterAnimationSystem::PrepareForRemove()
{
    using namespace DAVA;
}

void ShooterCharacterAnimationSystem::UpdateInputDependentParams(const DAVA::Entity* player, const DAVA::ActionsSingleComponent::Actions& actions, DAVA::float32 dt)
{
    using namespace DAVA;

    MotionComponent* motionComponent = player->GetComponent<MotionComponent>();
    DVASSERT(motionComponent != nullptr);

    dt *= motionComponent->GetPlaybackRate();

    Vector2 movingDirection;
    bool running = false;
    for (const FastName& action : actions.digitalActions)
    {
        if (action == SHOOTER_ACTION_MOVE_FORWARD)
        {
            movingDirection.y += 1.f;
        }
        else if (action == SHOOTER_ACTION_MOVE_BACKWARD)
        {
            movingDirection.y -= 1.f;
        }
        else if (action == SHOOTER_ACTION_MOVE_LEFT)
        {
            movingDirection.x -= 1.f;
        }
        else if (action == SHOOTER_ACTION_MOVE_RIGHT)
        {
            movingDirection.x += 1.f;
        }
        else if (action == SHOOTER_ACTION_ACCELERATE)
        {
            running = true;
        }
    }

    // Direction

    const float32 movingDirectionDt = dt * 5.0f;

    float32 previousDirectionX = motionComponent->GetParameter(MOTION_PARAM_DIRECTION_X);
    float32 previousDirectionY = motionComponent->GetParameter(MOTION_PARAM_DIRECTION_Y);
    float32 newDirectionX = previousDirectionX;
    float32 newDirectionY = previousDirectionY;

    if (previousDirectionX > movingDirection.x)
    {
        newDirectionX -= movingDirectionDt;
    }
    else if (previousDirectionX < movingDirection.x)
    {
        newDirectionX += movingDirectionDt;
    }

    if (previousDirectionY > movingDirection.y)
    {
        newDirectionY -= movingDirectionDt;
    }
    else if (previousDirectionY < movingDirection.y)
    {
        newDirectionY += movingDirectionDt;
    }

    if (Abs(newDirectionX) < movingDirectionDt)
    {
        newDirectionX = 0.f;
    }
    if (Abs(newDirectionY) < movingDirectionDt)
    {
        newDirectionY = 0.f;
    }

    newDirectionX = Clamp(newDirectionX, -1.0f, 1.0f);
    newDirectionY = Clamp(newDirectionY, -1.0f, 1.0f);

    motionComponent->SetParameter(MOTION_PARAM_DIRECTION_X, newDirectionX);
    motionComponent->SetParameter(MOTION_PARAM_DIRECTION_Y, newDirectionY);

    // Running

    const float32 runningDt = dt * 3.0f;

    const float32 previousRunning = motionComponent->GetParameter(MOTION_PARAM_RUNNING);
    float32 newRunning = previousRunning;
    if (newDirectionX != 0.0f || newDirectionY != 0.0f)
    {
        newRunning += (running ? runningDt : -runningDt);
        newRunning = Clamp(newRunning, 0.0f, 1.0f);
    }

    motionComponent->SetParameter(MOTION_PARAM_RUNNING, newRunning);
}

void ShooterCharacterAnimationSystem::UpdateInputIndependentParams(const DAVA::Entity* player, const DAVA::float32 dt)
{
    using namespace DAVA;

    MotionComponent* motionComponent = player->GetComponent<MotionComponent>();
    DVASSERT(motionComponent != nullptr);

    ShooterAimComponent* aimComponent = player->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent != nullptr);

    Vector3 defaultAimRayEnd = SHOOTER_AIM_OFFSET + SHOOTER_MAX_SHOOTING_DISTANCE * SHOOTER_CHARACTER_FORWARD;
    defaultAimRayEnd.Normalize();
    float32 defaultAngle = std::acos(SHOOTER_CHARACTER_FORWARD.DotProduct(defaultAimRayEnd));

    motionComponent->SetParameter(MOTION_PARAM_AIM_ANGLE, RadToDeg(-aimComponent->GetCurrentAngleX() + defaultAngle));
}

void ShooterCharacterAnimationSystem::UpdateEventsAndWeapon(DAVA::Entity* player)
{
    using namespace DAVA;

    MotionComponent* motionComponent = player->GetComponent<MotionComponent>();
    DVASSERT(motionComponent != nullptr);

    SkeletonComponent* skeletonComponent = player->GetComponent<SkeletonComponent>();
    DVASSERT(skeletonComponent != nullptr);

    const static FastName TRIGGER_MOVE("move");
    const static FastName TRIGGER_STOP("stop");

    if (motionComponent->GetParameter(MOTION_PARAM_DIRECTION_X) != 0.0f || motionComponent->GetParameter(MOTION_PARAM_DIRECTION_Y) != 0.0f)
    {
        motionComponent->TriggerEvent(TRIGGER_MOVE);
    }
    else
    {
        motionComponent->TriggerEvent(TRIGGER_STOP);
    }

    // Weapon transform

    uint32 weaponJointIndex = skeletonComponent->GetJointIndex(FastName("node-RH_WP"));
    DVASSERT(weaponJointIndex != SkeletonComponent::INVALID_JOINT_INDEX);

    const JointTransform& weaponJointTransform = skeletonComponent->GetJointObjectSpaceTransform(weaponJointIndex);
    Matrix4 weaponTransform =
    Matrix4::MakeRotation(Vector3::UnitX, -DegToRad(90.f)) *
    weaponJointTransform.GetOrientation().GetMatrix() *
    Matrix4::MakeTranslation(weaponJointTransform.GetPosition());

    Entity* weaponEntity = player->FindByName("Weapon");
    DVASSERT(weaponEntity != nullptr);
    weaponEntity->GetComponent<TransformComponent>()->SetLocalTransform(Transform(weaponTransform));
}