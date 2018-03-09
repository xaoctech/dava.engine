#include "Systems/ShooterPlayerMovementSystem.h"
#include "Components/ShooterRoleComponent.h"
#include "Components/ShooterAimComponent.h"
#include "Components/ShooterCarUserComponent.h"
#include "Components/ShooterMirroredCharacterComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
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

#include <NetworkCore/Snapshot.h>
#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/SnapshotUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>

#include <Physics/PhysicsSystem.h>
#include <Physics/PhysicsUtils.h>
#include <Physics/StaticBodyComponent.h>
#include <Physics/VehicleCarComponent.h>
#include <Physics/CharacterControllerComponent.h>
#include <Physics/CapsuleShapeComponent.h>
#include <Physics/Private/PhysicsMath.h>

#include <NetworkPhysics/CharacterMirrorsSingleComponent.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterMovementSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterMovementSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterMovementSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 13.0f)]
    .End();
}

ShooterMovementSystem::ShooterMovementSystem(DAVA::Scene* scene)
    : DAVA::BaseSimulationSystem(scene, DAVA::ComponentUtils::MakeMask<ShooterRoleComponent, DAVA::NetworkInputComponent>())
{
    using namespace DAVA;

    // TODO: get rid of these device getters

    Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
    uint32 keyboardId = ~0;
    if (kb != nullptr)
    {
        keyboardId = kb->GetId();
    }

    ActionsSingleComponent* actionsSingleComponent = scene->GetSingleComponentForWrite<ActionsSingleComponent>(this);
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_MOVE_FORWARD, eInputElements::KB_W, keyboardId);
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_MOVE_BACKWARD, eInputElements::KB_S, keyboardId);
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_MOVE_LEFT, eInputElements::KB_A, keyboardId);
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_MOVE_RIGHT, eInputElements::KB_D, keyboardId);
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_ACCELERATE, eInputElements::KB_LSHIFT, keyboardId);
    actionsSingleComponent->AddAvailableAnalogAction(SHOOTER_ACTION_ANALOG_MOVE, AnalogPrecision::ANALOG_UINT16);

    entityGroup = scene->AquireEntityGroup<ShooterRoleComponent, DAVA::NetworkInputComponent>();
}

void ShooterMovementSystem::ProcessFixed(DAVA::float32 dt)
{
    using namespace DAVA;

    // TODO: this should be implemented via class derived from PhysicsSystem. It cannot be done right now but should be possible later
    // For now use this workaround with signals
    static bool handledResolveCollisionMode = false;
    if (!handledResolveCollisionMode)
    {
        BattleOptionsSingleComponent* optionsSingleComponent = GetScene()->GetSingleComponent<BattleOptionsSingleComponent>();
        DVASSERT(optionsSingleComponent != nullptr);

        if (optionsSingleComponent->collisionResolveMode == COLLISION_RESOLVE_MODE_REWIND_IN_PAST)
        {
            PhysicsSystem* physicsSystem = GetScene()->GetSystem<PhysicsSystem>();
            DVASSERT(physicsSystem != nullptr);
            physicsSystem->beforeCCTMove.Connect(this, &ShooterMovementSystem::BeforeCharacterMove);
            physicsSystem->afterCCTMove.Connect(this, &ShooterMovementSystem::AfterCharacterMove);
        }

        handledResolveCollisionMode = true;
    }

    for (Entity* entity : entityGroup->GetEntities())
    {
        if (entity->GetComponent<ShooterRoleComponent>()->GetRole() != ShooterRoleComponent::Role::Player)
        {
            continue;
        }

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

void ShooterMovementSystem::ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration)
{
    using namespace DAVA;

    lastClientFrameId = clientFrameId;

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
                offset.y += -1.0f;
            }
            else if (action == SHOOTER_ACTION_MOVE_BACKWARD)
            {
                offset.y += 1.0f;
            }
            else if (action == SHOOTER_ACTION_MOVE_LEFT)
            {
                offset.x += 1.0f;
            }
            else if (action == SHOOTER_ACTION_MOVE_RIGHT)
            {
                offset.x += -1.0f;
            }
            else if (action == SHOOTER_ACTION_ACCELERATE)
            {
                accelerate = true;
            }
        }

        if (offset.SquareLength() > 0.0f)
        {
            offset = Normalize(offset) * SHOOTER_MOVEMENT_SPEED;

            if (accelerate)
            {
                offset *= 1.5f;
            }

            MoveCharacter(entity, offset);
        }
    }
}

void ShooterMovementSystem::ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration)
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

void ShooterMovementSystem::BeforeCharacterMove(DAVA::CharacterControllerComponent* controllerComponent)
{
    using namespace DAVA;

    // Collisions in the past implementation

    SnapshotSingleComponent* snapshotSingleComponent = GetScene()->GetSingleComponent<SnapshotSingleComponent>();
    DVASSERT(snapshotSingleComponent != nullptr);

    NetworkTimeSingleComponent* timeSingleComponent = GetScene()->GetSingleComponent<NetworkTimeSingleComponent>();
    DVASSERT(timeSingleComponent != nullptr);

    NetworkReplicationSingleComponent* replicationSingleComponent = GetScene()->GetSingleComponent<NetworkReplicationSingleComponent>();
    DVASSERT(replicationSingleComponent != nullptr);

    CharacterMirrorsSingleComponent* mirrorSingleComponent = GetScene()->GetSingleComponent<CharacterMirrorsSingleComponent>();
    DVASSERT(mirrorSingleComponent != nullptr);

    const DAVA::UnorderedMap<DAVA::Entity*, DAVA::Entity*>& characterToMirrorMap = mirrorSingleComponent->GetCharacterToMirrorMap();

    // If there are less than two players, no need to do anything
    if (characterToMirrorMap.size() < 2)
    {
        return;
    }

    NetworkReplicationComponent* movingPlayerReplicationComponent = controllerComponent->GetEntity()->GetComponent<NetworkReplicationComponent>();
    DVASSERT(movingPlayerReplicationComponent != nullptr);

    for (auto& pair : characterToMirrorMap)
    {
        Entity* cct = pair.first;
        DVASSERT(cct != nullptr);

        Entity* mirror = pair.second;
        DVASSERT(mirror != nullptr);

        ShooterMirroredCharacterComponent* mirroredCctComponent = cct->GetComponent<ShooterMirroredCharacterComponent>();
        DVASSERT(mirroredCctComponent != nullptr);

        if (mirroredCctComponent->GetMirrorIsMaster())
        {
            return;
        }

        // If it's not the player we're going to move
        if (cct != controllerComponent->GetEntity())
        {
            // Make the other player's mirror to collide with players
            CapsuleShapeComponent* capsuleShape = mirror->GetComponent<CapsuleShapeComponent>();
            DVASSERT(capsuleShape != nullptr);
            uint32 mask = capsuleShape->GetTypeMaskToCollideWith();
            mask |= SHOOTER_CHARACTER_COLLISION_TYPE;
            capsuleShape->SetTypeMaskToCollideWith(mask);

            NetworkReplicationComponent* enemyReplicationComponent = cct->GetComponent<NetworkReplicationComponent>();
            DVASSERT(enemyReplicationComponent != nullptr);

            if (IsServer(GetScene()))
            {
                // On server, we move the other player's mirror in the past

                const FastName& movingPlayerToken = GetScene()->GetSingleComponent<NetworkGameModeSingleComponent>()->GetToken(movingPlayerReplicationComponent->GetNetworkPlayerID());
                DVASSERT(movingPlayerToken.IsValid());

                int32 fdiff = timeSingleComponent->GetClientViewDelay(movingPlayerToken,
                                                                      lastClientFrameId);
                if (fdiff < 0)
                {
                    return;
                }

                Snapshot* snapshot = snapshotSingleComponent->GetServerSnapshot(lastClientFrameId - fdiff);
                if (snapshot == nullptr)
                {
                    return;
                }

                NetworkTransformComponent oldTransformComponent;

                SnapshotComponentKey componentKey(ComponentUtils::GetRuntimeId<NetworkTransformComponent>(), 0);
                bool applyResult = SnapshotUtils::ApplySnapshot(snapshot, enemyReplicationComponent->GetNetworkID(), componentKey, &oldTransformComponent);
                if (!applyResult)
                {
                    return;
                }

                mirror->GetComponent<DynamicBodyComponent>()->GetPxActor()->setGlobalPose(physx::PxTransform(PhysicsMath::Vector3ToPxVec3(oldTransformComponent.GetPosition()), physx::PxQuat(0.0f, 0.0f, 0.0f, 1.0f)));
            }
            else
            {
                // On client, we only move other player's mirror in the past if we're resimulating
                // Otherwise it's already in the past

                auto& historyMap = replicationInfoHistory[enemyReplicationComponent->GetNetworkID()];

                uint32 enemyFrameId = 0;
                if (replicationSingleComponent->replicationInfo.find(enemyReplicationComponent->GetNetworkID()) != replicationSingleComponent->replicationInfo.end())
                {
                    // Remember on which frame the enemy is right now
                    enemyFrameId = replicationSingleComponent->replicationInfo[enemyReplicationComponent->GetNetworkID()].frameIdServer;

                    // Save it to history so that we can use this value when resimulating
                    if (historyMap.size() == 0 || historyMap.rbegin()->first < timeSingleComponent->GetFrameId())
                    {
                        if (historyMap.size() >= 32)
                        {
                            historyMap.erase(historyMap.begin());
                        }

                        historyMap[timeSingleComponent->GetFrameId()] = enemyFrameId;
                    }
                }

                // Check if we have to rewind enemy to the past
                // We need to if current frame we see this enemy in does not match the frame we had seen this enemy before (i.e. when resimulating)
                bool rewind = false;
                for (auto it = historyMap.rbegin(); it != historyMap.rend(); ++it)
                {
                    if (it->first <= timeSingleComponent->GetFrameId())
                    {
                        if (it->second != enemyFrameId)
                        {
                            enemyFrameId = it->second;
                            rewind = true;
                        }

                        break;
                    }
                }

                // Rewind if neeeded. Otherwise just put mirror in the same place where enemy charater is right now

                if (rewind)
                {
                    Snapshot* snapshot = snapshotSingleComponent->GetServerSnapshot(enemyFrameId);
                    if (snapshot == nullptr)
                    {
                        return;
                    }

                    NetworkTransformComponent oldTransformComponent;

                    SnapshotComponentKey componentKey(ComponentUtils::GetRuntimeId<NetworkTransformComponent>(), 0);
                    bool applyResult = SnapshotUtils::ApplySnapshot(snapshot, enemyReplicationComponent->GetNetworkID(), componentKey, &oldTransformComponent);
                    if (!applyResult)
                    {
                        return;
                    }

                    mirror->GetComponent<DynamicBodyComponent>()->GetPxActor()->setGlobalPose(physx::PxTransform(PhysicsMath::Vector3ToPxVec3(oldTransformComponent.GetPosition()), physx::PxQuat(0.0f, 0.0f, 0.0f, 1.0f)));
                }
                else
                {
                    TransformComponent* cctTransformComponent = cct->GetComponent<TransformComponent>();
                    mirror->GetComponent<DynamicBodyComponent>()->GetPxActor()->setGlobalPose(physx::PxTransform(PhysicsMath::Vector3ToPxVec3(cctTransformComponent->GetPosition()), physx::PxQuat(0.0f, 0.0f, 0.0f, 1.0f)));
                }
            }
        }
    }
}

void ShooterMovementSystem::AfterCharacterMove(DAVA::CharacterControllerComponent* controllerComponent)
{
    using namespace DAVA;

    // Collisions in the past implementation

    CharacterMirrorsSingleComponent* mirrorSingleComponent = GetScene()->GetSingleComponent<CharacterMirrorsSingleComponent>();
    DVASSERT(mirrorSingleComponent != nullptr);

    const DAVA::UnorderedMap<DAVA::Entity*, DAVA::Entity*>& characterToMirrorMap = mirrorSingleComponent->GetCharacterToMirrorMap();

    for (auto& pair : characterToMirrorMap)
    {
        Entity* cct = pair.first;
        DVASSERT(cct != nullptr);

        Entity* mirror = pair.second;
        DVASSERT(mirror != nullptr);

        ShooterMirroredCharacterComponent* mirroredCctComponent = cct->GetComponent<ShooterMirroredCharacterComponent>();
        DVASSERT(mirroredCctComponent != nullptr);

        if (mirroredCctComponent->GetMirrorIsMaster())
        {
            return;
        }

        if (cct != controllerComponent->GetEntity())
        {
            // Disable mirror's collisions with players
            CapsuleShapeComponent* capsuleShape = mirror->GetComponent<CapsuleShapeComponent>();
            DVASSERT(capsuleShape != nullptr);
            uint32 mask = capsuleShape->GetTypeMaskToCollideWith();
            mask &= ~SHOOTER_CHARACTER_COLLISION_TYPE;
            capsuleShape->SetTypeMaskToCollideWith(mask);
        }
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
