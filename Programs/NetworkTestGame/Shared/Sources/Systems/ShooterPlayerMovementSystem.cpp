#include "Systems/ShooterPlayerMovementSystem.h"
#include "Components/ShooterRoleComponent.h"
#include "Components/ShooterAimComponent.h"
#include "Components/ShooterCarUserComponent.h"
#include "ShooterConstants.h"
#include "ShooterUtils.h"

#include <Entity/ComponentUtils.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Engine/Window.h>
#include <DeviceManager/DeviceManager.h>
#include <Input/Keyboard.h>
#include <Input/Mouse.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/ActionCollectSystem.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Debug/ProfilerCPU.h>

#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/NetworkCoreUtils.h>
#include <Physics/PhysicsUtils.h>
#include <Physics/StaticBodyComponent.h>
#include <Physics/VehicleCarComponent.h>
#include <Physics/CharacterControllerComponent.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterMovementSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterMovementSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterMovementSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 13.0f)]
    .End();
}

ShooterMovementSystem::ShooterMovementSystem(DAVA::Scene* scene)
    : DAVA::INetworkInputSimulationSystem(scene, DAVA::ComponentUtils::MakeMask<ShooterRoleComponent>() | DAVA::ComponentUtils::MakeMask<DAVA::NetworkInputComponent>())
{
    using namespace DAVA;

    // TODO: get rid of these device getters

    Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
    uint32 keyboardId = ~0;
    if (kb != nullptr)
    {
        keyboardId = kb->GetId();
    }

    ActionsSingleComponent* actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_MOVE_FORWARD, eInputElements::KB_W, keyboardId);
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_MOVE_BACKWARD, eInputElements::KB_S, keyboardId);
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_MOVE_LEFT, eInputElements::KB_A, keyboardId);
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_MOVE_RIGHT, eInputElements::KB_D, keyboardId);
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_ACCELERATE, eInputElements::KB_LSHIFT, keyboardId);
    actionsSingleComponent->AddAvailableAnalogAction(SHOOTER_ACTION_ANALOG_MOVE, AnalogPrecision::ANALOG_UINT16);
}

void ShooterMovementSystem::AddEntity(DAVA::Entity* entity)
{
    ShooterRoleComponent* roleComponent = entity->GetComponent<ShooterRoleComponent>();
    DVASSERT(roleComponent != nullptr);

    if (roleComponent->GetRole() == ShooterRoleComponent::Role::Player)
    {
        playerEntities.insert(entity);
    }
}

void ShooterMovementSystem::RemoveEntity(DAVA::Entity* entity)
{
    playerEntities.erase(entity);
}

void ShooterMovementSystem::ProcessFixed(DAVA::float32 dt)
{
    using namespace DAVA;

    for (Entity* entity : playerEntities)
    {
        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), entity);
        if (!allActions.empty())
        {
            const auto& actions = allActions.back();
            ApplyDigitalActions(entity, actions.digitalActions, actions.clientFrameId, dt);
            ApplyAnalogActions(entity, actions.analogActions, actions.clientFrameId, dt);
        }

        RotateEntityTowardsCurrentAim(entity);
    }
}

void ShooterMovementSystem::PrepareForRemove()
{
}

void ShooterMovementSystem::Simulate(DAVA::Entity* entity)
{
    DAVA::INetworkInputSimulationSystem::Simulate(entity);
    RotateEntityTowardsCurrentAim(entity);
}

void ShooterMovementSystem::ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const
{
    using namespace DAVA;

    DAVA_PROFILER_CPU_SCOPE("ShooterMovementSystem::ProcessFixed");

    DVASSERT(IsServer(GetScene()) || IsClientOwner(GetScene(), entity));

    ShooterRoleComponent* roleComponent = entity->GetComponent<ShooterRoleComponent>();
    DVASSERT(roleComponent != nullptr);
    DVASSERT(roleComponent->GetRole() == ShooterRoleComponent::Role::Player);

    ShooterCarUserComponent* carUserComponent = entity->GetComponent<ShooterCarUserComponent>();
    DVASSERT(carUserComponent != nullptr);

    // Driving a car
    if (carUserComponent->GetCarNetworkId() != NetworkID::INVALID && IsServer(GetScene()) && carUserComponent->GetPassengerIndex() == 0)
    {
        Entity* carEntity = GetEntityWithNetworkId(GetScene(), carUserComponent->GetCarNetworkId());
        VehicleCarComponent* car = carEntity->GetComponent<VehicleCarComponent>();
        if (car != nullptr)
        {
            float32 acceleration = 0.0f;
            float32 steer = 0.0f;

            for (const FastName& action : actions)
            {
                if (action == SHOOTER_ACTION_MOVE_FORWARD)
                {
                    acceleration += 1.0f;
                }
                else if (action == SHOOTER_ACTION_MOVE_BACKWARD)
                {
                    acceleration -= 1.0f;
                }
                else if (action == SHOOTER_ACTION_MOVE_LEFT)
                {
                    steer += 1.0f;
                }
                else if (action == SHOOTER_ACTION_MOVE_RIGHT)
                {
                    steer -= 1.0f;
                }
            }

            MoveCar(car, acceleration, steer);
        }
    }
    // Moving character
    else
    {
        Vector3 offset = Vector3::Zero;
        bool accelerate = false;

        for (const FastName& action : actions)
        {
            if (action == SHOOTER_ACTION_MOVE_FORWARD)
            {
                offset.y += -SHOOTER_MOVEMENT_SPEED;
            }
            else if (action == SHOOTER_ACTION_MOVE_BACKWARD)
            {
                offset.y += SHOOTER_MOVEMENT_SPEED;
            }
            else if (action == SHOOTER_ACTION_MOVE_LEFT)
            {
                offset.x += SHOOTER_MOVEMENT_SPEED;
            }
            else if (action == SHOOTER_ACTION_MOVE_RIGHT)
            {
                offset.x += -SHOOTER_MOVEMENT_SPEED;
            }
            else if (action == SHOOTER_ACTION_ACCELERATE)
            {
                accelerate = true;
            }
        }

        if (accelerate)
        {
            offset *= 1.5f;
        }

        MoveCharacter(entity, offset);
    }
}

void ShooterMovementSystem::ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const
{
    using namespace DAVA;

    ShooterAimComponent* aimComponent = entity->GetComponent<ShooterAimComponent>();

    for (const auto& action : actions)
    {
        Vector2 analogActionState = ConvertFixedPrecisionToAnalog(action.first.precision, action.second);

        if (action.first.actionId == SHOOTER_ACTION_ANALOG_MOVE)
        {
            ShooterCarUserComponent* carUserComponent = entity->GetComponent<ShooterCarUserComponent>();
            DVASSERT(carUserComponent != nullptr);

            // Driving a car
            if (carUserComponent->GetCarNetworkId() != NetworkID::INVALID && IsServer(GetScene()) && carUserComponent->GetPassengerIndex() == 0)
            {
                Entity* carEntity = GetEntityWithNetworkId(GetScene(), carUserComponent->GetCarNetworkId());
                VehicleCarComponent* car = carEntity->GetComponent<VehicleCarComponent>();
                if (car != nullptr)
                {
                    MoveCar(car, -analogActionState.y, -analogActionState.x);
                }
            }
            else
            {
                if (std::abs(analogActionState.x) > 0.9f || std::abs(analogActionState.y) > 0.9f)
                {
                    // Accelerate
                    analogActionState *= 1.5f;
                }

                MoveCharacter(entity, Vector3(-analogActionState.x * SHOOTER_MOVEMENT_SPEED, analogActionState.y * SHOOTER_MOVEMENT_SPEED, 0.0f));
            }
        }
    }
}

void ShooterMovementSystem::RotateEntityTowardsCurrentAim(DAVA::Entity* entity)
{
    using namespace DAVA;

    ShooterAimComponent* aimComponent = entity->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent != nullptr);

    // Get current aim ray
    Vector3 aimRayStart;
    Vector3 aimRayDirection;
    Vector3 aimRayEnd;
    Entity* aimRayEndEntity;
    GetCurrentAimRay(*aimComponent, RaycastFilter::IGNORE_SOURCE | RaycastFilter::IGNORE_DYNAMICS, aimRayStart, aimRayDirection, aimRayEnd, &aimRayEndEntity);

    // Create quaternion that represents partial rotation towards current aim ray
    TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
    DVASSERT(transformComponent != nullptr);

    const Quaternion& currentOrientation = transformComponent->GetRotation();

    Vector3 currentForward = currentOrientation.ApplyToVectorFast(SHOOTER_CHARACTER_FORWARD);
    Vector3 finalForward = aimRayEnd - transformComponent->GetPosition();
    finalForward.Normalize();

    Quaternion deltaOrientation = Quaternion::MakeRotation(currentForward, finalForward);
    deltaOrientation.x = deltaOrientation.y = 0.0f; // Ignore rotation around X and Y axes, since we rotate character only around Z
    deltaOrientation.Normalize();

    if (FLOAT_EQUAL(deltaOrientation.z, 0.0f) && FLOAT_EQUAL(deltaOrientation.w, 1.0f))
    {
        return;
    }

    Quaternion intermediateOrientation;
    intermediateOrientation.Slerp(currentOrientation, currentOrientation * deltaOrientation, SHOOTER_CHARACTER_ROTATION_SPEED);
    intermediateOrientation.Normalize();

    // Rotate
    if (!FLOAT_EQUAL(intermediateOrientation.x, currentOrientation.x) ||
        !FLOAT_EQUAL(intermediateOrientation.y, currentOrientation.y) ||
        !FLOAT_EQUAL(intermediateOrientation.z, currentOrientation.z) ||
        !FLOAT_EQUAL(intermediateOrientation.w, currentOrientation.w))
    {
        transformComponent->SetLocalTransform(transformComponent->GetPosition(), intermediateOrientation, transformComponent->GetScale());
    }
}

void ShooterMovementSystem::MoveCharacter(DAVA::Entity* player, const DAVA::Vector3& offset) const
{
    using namespace DAVA;

    if (offset != Vector3::Zero)
    {
        CharacterControllerComponent* characterControllerComponent = PhysicsUtils::GetCharacterControllerComponent(player);
        if (characterControllerComponent != nullptr)
        {
            TransformComponent* transformComponent = player->GetComponent<TransformComponent>();
            characterControllerComponent->Move(transformComponent->GetRotation().ApplyToVectorFast(offset));
        }
    }
}

void ShooterMovementSystem::MoveCar(DAVA::VehicleCarComponent* car, DAVA::float32 acceleration, DAVA::float32 steer) const
{
    using namespace DAVA;

    car->SetAnalogSteer(Clamp(steer, -1.0f, 1.0f));

    if (acceleration < 0.0f)
    {
        car->SetGear(eVehicleGears::Reverse);
    }
    else if (acceleration != 0.0f)
    {
        car->SetGear(eVehicleGears::First);
    }

    car->SetAnalogAcceleration(std::abs(Clamp(acceleration, -1.0f, 1.0f)));
}
