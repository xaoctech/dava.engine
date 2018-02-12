#include "Systems/ShooterCarSystem.h"
#include "Components/ShooterAimComponent.h"
#include "Components/ShooterRoleComponent.h"
#include "Components/ShooterCarUserComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "ShooterUtils.h"
#include "ShooterConstants.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Input/Keyboard.h>
#include <DeviceManager/DeviceManager.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Systems/ActionCollectSystem.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <UI/UIControlSystem.h>
#include <UI/UIControl.h>
#include <UI/Text/UITextComponent.h>
#include <Utils/StringFormat.h>
#include <Debug/ProfilerCPU.h>

#include <Physics/CapsuleCharacterControllerComponent.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/Private/PhysicsMath.h>
#include <Physics/PhysicsUtils.h>

#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/NetworkCoreUtils.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterCarSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterCarSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterCarSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 16.0f)]
    .End();
}

ShooterCarSystem::ShooterCarSystem(DAVA::Scene* scene)
    : DAVA::INetworkInputSimulationSystem(scene, DAVA::ComponentUtils::MakeMask<ShooterAimComponent>())
    , interactionControl(nullptr)
{
    using namespace DAVA;

    // TODO: get rid of these identifiers
    Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
    uint32 kbId = ~0;
    if (kb != nullptr)
    {
        kbId = kb->GetId();
    }

    ActionsSingleComponent* actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_INTERACT, eInputElements::KB_E, kbId, DigitalElementState::JustPressed());

    BattleOptionsSingleComponent* battleOptionsSingleComponent = scene->GetSingletonComponent<BattleOptionsSingleComponent>();

    DVASSERT(battleOptionsSingleComponent->isSet);

    interactionControl = battleOptionsSingleComponent->controls.interactControl;
}

void ShooterCarSystem::AddEntity(DAVA::Entity* entity)
{
    ShooterAimComponent* aimComponent = entity->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent != nullptr);

    aimComponents.insert(aimComponent);
}

void ShooterCarSystem::RemoveEntity(DAVA::Entity* entity)
{
    ShooterAimComponent* aimComponent = entity->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent != nullptr);

    aimComponents.erase(aimComponent);
}

void ShooterCarSystem::ProcessFixed(DAVA::float32 dt)
{
    using namespace DAVA;

    DAVA_PROFILER_CPU_SCOPE("ShooterCarSystem::ProcessFixed");

    for (ShooterAimComponent* aimComponent : aimComponents)
    {
        DVASSERT(aimComponent != nullptr);

        Entity* aimingEntity = aimComponent->GetEntity();
        DVASSERT(aimingEntity != nullptr);

        if (!IsServer(this) && IsClientOwner(this, aimingEntity))
        {
            UpdateInteractionControl(aimComponent);
        }

        if (IsServer(this) || IsClientOwner(this, aimingEntity))
        {
            ToggleCharacterStateIfRequired(aimingEntity);
        }

        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), aimingEntity);
        if (!allActions.empty())
        {
            const auto& actions = allActions.back();
            ApplyDigitalActions(aimingEntity, actions.digitalActions, actions.clientFrameId, dt);
            ApplyAnalogActions(aimingEntity, actions.analogActions, actions.clientFrameId, dt);
        }
    }
}

void ShooterCarSystem::PrepareForRemove()
{
}

void ShooterCarSystem::ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const
{
    using namespace DAVA;

    ShooterAimComponent* aimComponent = entity->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent != nullptr);

    for (const FastName& action : actions)
    {
        if (action == SHOOTER_ACTION_INTERACT)
        {
            if (IsServer(GetScene()))
            {
                ShooterCarUserComponent* carUserComponent = entity->GetComponent<ShooterCarUserComponent>();
                DVASSERT(carUserComponent != nullptr);

                // If player is outside of a car, put him in
                // Otherwise get him out
                if (carUserComponent->carNetworkId == NetworkID::INVALID)
                {
                    // If there is a car he points to
                    Entity* interactionEntity = GetTargetCar(aimComponent);
                    if (interactionEntity != nullptr)
                    {
                        ShooterRoleComponent* roleComponent = interactionEntity->GetComponent<ShooterRoleComponent>();
                        if (roleComponent != nullptr && roleComponent->GetRole() == ShooterRoleComponent::Role::Car)
                        {
                            DynamicBodyComponent* carDynamicBodyComponent = interactionEntity->GetComponent<DynamicBodyComponent>();
                            DVASSERT(carDynamicBodyComponent != nullptr);

                            // If car is not moving too fast...
                            if (carDynamicBodyComponent->GetLinearVelocity().Length() < 7.0f)
                            {
                                CarInfo carInfo = GetCarInfo(interactionEntity);

                                // ...and if it's not full, put him in
                                if (carInfo.numPassengers < SHOOTER_MAX_NUM_PASSENGERS)
                                {
                                    NetworkReplicationComponent* carReplicationComponent = interactionEntity->GetComponent<NetworkReplicationComponent>();
                                    DVASSERT(carReplicationComponent != nullptr);

                                    carUserComponent->carNetworkId = carReplicationComponent->GetNetworkID();
                                    carUserComponent->passengerIndex = carInfo.firstFreeIndex;

                                    // Teleport player under the ground for now
                                    // TODO: change hierarchy so that player is a child of the car. Can't do that right now since replication system fails when hierarchy changes
                                    TransformComponent* carUserTransformComponent = entity->GetComponent<TransformComponent>();
                                    carUserTransformComponent->SetLocalTransform(Vector3(0.0f, 0.0f, 0.0f), carUserTransformComponent->GetRotation(), carUserTransformComponent->GetScale());
                                }
                            }
                        }
                    }
                }
                else
                {
                    Entity* car = GetEntityWithNetworkId(GetScene(), carUserComponent->carNetworkId);
                    DVASSERT(car != nullptr);

                    // Calculate position to position character when he gets out
                    Vector3 nodePositionLocalSpace = SHOOTER_CAR_PASSENGER_NODES_POSITIONS[carUserComponent->passengerIndex];
                    Vector3 nodePositionWorldSpace = nodePositionLocalSpace * car->GetWorldTransform();
                    physx::PxRaycastHit hit;
                    QueryFilterCallback filterCallback(nullptr, RaycastFilter::NONE);
                    bool collision = GetRaycastHit(*GetScene(), Vector3(nodePositionWorldSpace.x, nodePositionWorldSpace.y, nodePositionWorldSpace.z), Vector3(0.0f, 0.0f, -1.0f), 100.0f, &filterCallback, hit);
                    if (collision)
                    {
                        nodePositionWorldSpace = PhysicsMath::PxVec3ToVector3(hit.position);
                    }

                    // Position him
                    TransformComponent* carUserTransformComponent = entity->GetComponent<TransformComponent>();
                    carUserTransformComponent->SetLocalTransform(nodePositionWorldSpace, carUserTransformComponent->GetRotation(), carUserTransformComponent->GetScale());

                    // Reset his aim angle around X axis
                    aimComponent->SetFinalAngleX(0.0f);
                    aimComponent->SetCurrentAngleX(0.0f);

                    // Remember he's not in the car anymore
                    carUserComponent->carNetworkId = NetworkID::INVALID;
                }
            }
        }
    }
}

void ShooterCarSystem::ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const
{
}

void ShooterCarSystem::SetInteractionControl(DAVA::UIControl* value)
{
    interactionControl = value;
}

DAVA::Entity* ShooterCarSystem::GetParentCar(DAVA::Entity* player) const
{
    using namespace DAVA;

    DVASSERT(player != nullptr);

    ShooterCarUserComponent* carUserComponent = player->GetComponent<ShooterCarUserComponent>();
    DVASSERT(carUserComponent != nullptr);

    if (carUserComponent->carNetworkId != NetworkID::INVALID)
    {
        Entity* car = GetEntityWithNetworkId(GetScene(), carUserComponent->carNetworkId);
        DVASSERT(car != nullptr);

        return car;
    }

    return nullptr;
}

DAVA::Entity* ShooterCarSystem::GetTargetCar(ShooterAimComponent* aimComponent) const
{
    using namespace DAVA;

    Vector3 aimRayOrigin;
    Vector3 aimRayDirection;
    Vector3 aimRayEnd;
    Entity* aimRayEndEntity;
    GetCurrentAimRay(*aimComponent, RaycastFilter::IGNORE_SOURCE, aimRayOrigin, aimRayDirection, aimRayEnd, &aimRayEndEntity);

    // If we point to some entity, check if it's a car and we're close enough
    if (aimRayEndEntity != nullptr)
    {
        ShooterRoleComponent* roleComponent = aimRayEndEntity->GetComponent<ShooterRoleComponent>();
        if (roleComponent != nullptr && roleComponent->GetRole() == ShooterRoleComponent::Role::Car)
        {
            Entity* aimingEntity = aimComponent->GetEntity();
            DVASSERT(aimingEntity != nullptr);

            TransformComponent* aimingEntityTransformComponent = aimingEntity->GetComponent<TransformComponent>();
            DVASSERT(aimingEntityTransformComponent != nullptr);

            TransformComponent* targetTransformComponent = aimRayEndEntity->GetComponent<TransformComponent>();
            DVASSERT(targetTransformComponent != nullptr);

            if (Distance(targetTransformComponent->GetWorldTransform().GetTranslationVector(), aimingEntityTransformComponent->GetWorldTransform().GetTranslationVector()) < 3.5f)
            {
                return aimRayEndEntity;
            }
        }
    }

    return nullptr;
}

bool ShooterCarSystem::IsDriver(DAVA::Entity* player) const
{
    using namespace DAVA;

    DVASSERT(player != nullptr);

    ShooterCarUserComponent* carUserComponent = player->GetComponent<ShooterCarUserComponent>();
    DVASSERT(carUserComponent != nullptr);

    return (carUserComponent->passengerIndex == 0);
}

ShooterCarSystem::CarInfo ShooterCarSystem::GetCarInfo(DAVA::Entity* car) const
{
    using namespace DAVA;

    DVASSERT(car != nullptr);

    NetworkReplicationComponent* carReplicationComponent = car->GetComponent<NetworkReplicationComponent>();
    DVASSERT(carReplicationComponent != nullptr);

    Vector<Entity*> carUsers;
    GetScene()->GetChildEntitiesWithComponent(carUsers, Type::Instance<ShooterCarUserComponent>());

    uint32 numPassengers = 0;
    bool slots[SHOOTER_MAX_NUM_PASSENGERS] = { 0 };
    for (Entity* user : carUsers)
    {
        DVASSERT(user != nullptr);

        ShooterCarUserComponent* carUserComponent = user->GetComponent<ShooterCarUserComponent>();
        DVASSERT(carUserComponent != nullptr);

        if (carUserComponent->GetCarNetworkId() == carReplicationComponent->GetNetworkID())
        {
            ++numPassengers;
            slots[carUserComponent->passengerIndex] = true;
        }
    }

    CarInfo resultInfo;
    resultInfo.numPassengers = numPassengers;
    resultInfo.firstFreeIndex = static_cast<uint32>(std::distance(slots, std::find(std::begin(slots), std::end(slots), false)));

    return resultInfo;
}

void ShooterCarSystem::UpdateInteractionControl(ShooterAimComponent* aimComponent)
{
    using namespace DAVA;

    if (interactionControl == nullptr)
    {
        return;
    }

    Entity* parentCar = GetParentCar(aimComponent->GetEntity());
    if (parentCar != nullptr)
    {
        VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
        Size2i windowSize = vcs->GetVirtualScreenSize();
        interactionControl->SetPosition(Vector2(windowSize.dx / 2.0f, 40.0f));

        interactionControl->SetVisibilityFlag(true);

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
        static const String GET_OUT_BUTTON = "B";
#else
        static const String GET_OUT_BUTTON = "(E)";
#endif

        CarInfo carInfo = GetCarInfo(parentCar);
        bool isDriver = IsDriver(aimComponent->GetEntity());

        // Cache strings
        static UnorderedMap<uint32, String> strings;
        uint32 params = carInfo.numPassengers + (isDriver ? (1 << 31) : 0);
        if (strings.find(params) == strings.end())
        {
            strings[params] = Format(
            "Press %s to get out of the car\n%s\nNumber of players inside: %u",
            GET_OUT_BUTTON.c_str(),
            isDriver ? "You're a driver" : "You're a passenger",
            carInfo.numPassengers);
        }

        interactionControl->GetComponent<UITextComponent>()->SetText(strings[params]);
    }
    else
    {
        Entity* targetCar = GetTargetCar(aimComponent);

        if (targetCar != nullptr)
        {
            const AABBox3 bbox = targetCar->GetWTMaximumBoundingBoxSlow();
            Vector3 uiWorldPosition = bbox.GetCenter() + Vector3(0.0f, 0.0f, bbox.GetSize().z / 2.0f);

            Camera* camera = GetScene()->GetCamera(0);
            DVASSERT(camera != nullptr);

            VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
            Size2i windowSize = vcs->GetVirtualScreenSize();

            Vector2 position = camera->GetOnScreenPosition(uiWorldPosition, Rect(0.0f, 0.0f, static_cast<float32>(windowSize.dx), static_cast<float32>(windowSize.dy)));
            position.y += 20.0f;
            interactionControl->SetPosition(position);

            interactionControl->SetVisibilityFlag(true);

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
            static const String GET_IN_TEXT = "Press 'B' button to get in the car";
#else
            static const String GET_IN_TEXT = "Press (E) to get in the car";
#endif
            interactionControl->GetComponent<UITextComponent>()->SetText(GET_IN_TEXT);
        }
        else
        {
            interactionControl->SetVisibilityFlag(false);
        }
    }
}

void ShooterCarSystem::ToggleCharacterStateIfRequired(DAVA::Entity* player) const
{
    using namespace DAVA;

    DVASSERT(player != nullptr);

    DVASSERT(IsServer(GetScene()) || IsClientOwner(GetScene(), player));

    ShooterCarUserComponent* carUserComponent = player->GetComponent<ShooterCarUserComponent>();
    DVASSERT(carUserComponent != nullptr);

    if (carUserComponent->carNetworkId != NetworkID::INVALID)
    {
        if (player->GetComponent<CapsuleCharacterControllerComponent>() != nullptr)
        {
            // Remove CCT
            // Will be recreated when player gets out
            player->RemoveComponent<CapsuleCharacterControllerComponent>();
        }
    }
    else if (player->GetComponent<CapsuleCharacterControllerComponent>() == nullptr)
    {
        // Recreate CCT once we got out
        CapsuleCharacterControllerComponent* controllerComponent = new CapsuleCharacterControllerComponent();
        controllerComponent->SetHeight(SHOOTER_CHARACTER_CAPSULE_HEIGHT);
        controllerComponent->SetRadius(SHOOTER_CHARACTER_CAPSULE_RADIUS);
        controllerComponent->SetTypeMask(SHOOTER_CHARACTER_COLLISION_TYPE);
        controllerComponent->SetTypeMaskToCollideWith(UINT32_MAX);
        player->AddComponent(controllerComponent);
    }
}
