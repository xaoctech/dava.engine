#include "Systems/ShooterPlayerAttackSystem.h"
#include "Components/ShooterRoleComponent.h"
#include "Components/ShooterAimComponent.h"
#include "Components/HealthComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Components/ShooterRocketComponent.h"
#include "Components/ShooterStateComponent.h"
#include "Components/RocketSpawnComponent.h"
#include "ShooterConstants.h"
#include "ShooterUtils.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <DeviceManager/DeviceManager.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/ActionCollectSystem.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Input/Mouse.h>
#include <Math/Vector.h>
#include <Math/Quaternion.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/Scene3D/Systems/NetworkIdSystem.h>
#include <NetworkPhysics/NetworkPhysicsUtils.h>
#include <NetworkPhysics/CharacterMirrorsSingleComponent.h>
#include <Physics/PhysicsSystem.h>
#include <Physics/CapsuleCharacterControllerComponent.h>
#include <Physics/Private/PhysicsMath.h>
#include <Physics/DynamicBodyComponent.h>
#include <Debug/ProfilerCPU.h>

#include <physx/PxRigidActor.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterPlayerAttackSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterPlayerAttackSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterPlayerAttackSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 14.0f)]
    .End();
}

ShooterPlayerAttackSystem::ShooterPlayerAttackSystem(DAVA::Scene* scene)
    : DAVA::INetworkInputSimulationSystem(scene, DAVA::ComponentUtils::MakeMask<ShooterRoleComponent>() | DAVA::ComponentUtils::MakeMask<ShooterStateComponent>())
{
    using namespace DAVA;

    // TODO: get rid of these identifiers
    Mouse* kb = GetEngineContext()->deviceManager->GetMouse();
    uint32 mouseId = ~0;
    if (kb != nullptr)
    {
        mouseId = kb->GetId();
    }

    ActionsSingleComponent* actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();
    actionsSingleComponent->AddAvailableDigitalAction(SHOOTER_ACTION_ATTACK_BULLET);
    actionsSingleComponent->AddAvailableDigitalAction(SHOOTER_ACTION_INTERACT);
    actionsSingleComponent->AddAvailableDigitalAction(SHOOTER_ACTION_ATTACK_ROCKET);

    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_ATTACK_BULLET, eInputElements::MOUSE_LBUTTON, mouseId, DigitalElementState::JustPressed());
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_ATTACK_ROCKET, eInputElements::MOUSE_RBUTTON, mouseId, DigitalElementState::Pressed());
}

void ShooterPlayerAttackSystem::AddEntity(DAVA::Entity* entity)
{
    ShooterRoleComponent* roleComponent = entity->GetComponent<ShooterRoleComponent>();
    DVASSERT(roleComponent != nullptr);

    if (roleComponent->GetRole() == ShooterRoleComponent::Role::Player)
    {
        playerEntities.insert(entity);
    }
}

void ShooterPlayerAttackSystem::RemoveEntity(DAVA::Entity* entity)
{
    playerEntities.erase(entity);
}

void ShooterPlayerAttackSystem::ProcessFixed(DAVA::float32 dt)
{
    using namespace DAVA;

    DAVA_PROFILER_CPU_SCOPE("ShooterPlayerAttackSystem::ProcessFixed");

    for (Entity* entity : playerEntities)
    {
        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), entity);
        for (const auto& actions : allActions)
        {
            ApplyDigitalActions(entity, actions.digitalActions, actions.clientFrameId, dt);
            ApplyAnalogActions(entity, actions.analogActions, actions.clientFrameId, dt);
        }
    }
}

void ShooterPlayerAttackSystem::PrepareForRemove()
{
}

void ShooterPlayerAttackSystem::ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const
{
    using namespace DAVA;

    ShooterRoleComponent* roleComponent = entity->GetComponent<ShooterRoleComponent>();
    DVASSERT(roleComponent != nullptr);
    DVASSERT(roleComponent->GetRole() == ShooterRoleComponent::Role::Player);

    bool hasActionAttackRocket = false;

    for (const FastName& action : actions)
    {
        if (action == SHOOTER_ACTION_ATTACK_BULLET)
        {
            // For ballistics shooting
            // SpawnBullet(entity, clientFrameId);

            // For raycast shooting
            RaycastAttack(entity, clientFrameId);
        }

        hasActionAttackRocket |= (action == SHOOTER_ACTION_ATTACK_ROCKET);
    }

    if (IsServer(GetScene()) || IsClientOwner(entity))
    {
        const HealthComponent* healthComp = entity->GetComponent<HealthComponent>();
        const bool isDamaged = (clientFrameId - healthComp->GetLastDamageId() < 2);
        RocketSpawnComponent* rocketSpawnComp = entity->GetComponent<RocketSpawnComponent>();
        uint32& rocketSpawnProgress = rocketSpawnComp->progress;
        if (hasActionAttackRocket && !isDamaged && rocketSpawnProgress < RocketSpawnComponent::THRESHOLD)
        {
            ++rocketSpawnProgress;
            if (rocketSpawnProgress == RocketSpawnComponent::THRESHOLD)
            {
                RocketAttack(entity, clientFrameId);
            }
        }
        else
        {
            rocketSpawnProgress = 0;
        }
    }
}

void ShooterPlayerAttackSystem::ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const
{
}

void ShooterPlayerAttackSystem::SpawnBullet(DAVA::Entity* player, DAVA::uint32 clientFrameId) const
{
    using namespace DAVA;

    DVASSERT(player != nullptr);

    ShootCooldownComponent* shootCooldownComponent = player->GetComponent<ShootCooldownComponent>();
    uint32 lastShootFrameId = shootCooldownComponent->GetLastShootFrameId();
    if (clientFrameId - lastShootFrameId < SHOOTER_SHOOT_COOLDOWN_FRAMES)
    {
        return;
    }

    shootCooldownComponent->SetLastShootFrameId(clientFrameId);

    // Spawn bullet

    Entity* bullet = new Entity();
    bullet->SetName("Bullet");

    ShooterRoleComponent* roleComponent = new ShooterRoleComponent();
    roleComponent->SetRole(ShooterRoleComponent::Role::Bullet);
    bullet->AddComponent(roleComponent);

    NetworkReplicationComponent* playerNetworkReplicationComponent = player->GetComponent<NetworkReplicationComponent>();
    DVASSERT(playerNetworkReplicationComponent != nullptr);

    uint32 playerId = playerNetworkReplicationComponent->GetNetworkPlayerID();

    NetworkReplicationComponent* bulletNetworkReplicationComponent = new NetworkReplicationComponent();
    bulletNetworkReplicationComponent->SetNetworkPlayerID(playerId);
    bullet->AddComponent(bulletNetworkReplicationComponent);

    NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent();
    networkPredictComponent->AddPredictedComponent(Type::Instance<NetworkTransformComponent>());
    networkPredictComponent->SetFrameActionID(FrameActionID(clientFrameId, playerId, 0));
    bullet->AddComponent(networkPredictComponent);

    // TODO: a better way to position the bullet?

    Entity* weaponBarrelEntity = player->FindByName(SHOOTER_GUN_BARREL_ENTITY_NAME);
    DVASSERT(weaponBarrelEntity != nullptr);

    Vector3 barrelPosition;
    Vector3 barrelScale;
    Quaternion barrelOrientation;
    const Matrix4& weaponBarrelTransform = weaponBarrelEntity->GetWorldTransform();
    weaponBarrelTransform.Decomposition(barrelPosition, barrelScale, barrelOrientation);

    TransformComponent* bulletTransformComponent = bullet->GetComponent<TransformComponent>();
    DVASSERT(bulletTransformComponent != nullptr);

    bulletTransformComponent->SetLocalTransform(barrelPosition, barrelOrientation, Vector3(1.0f, 1.0f, 1.0f));

    GetScene()->AddNode(bullet);
}

void ShooterPlayerAttackSystem::RaycastAttack(DAVA::Entity* aimingEntity, DAVA::uint32 clientFrameId) const
{
    using namespace DAVA;

    DVASSERT(aimingEntity != nullptr);

    ShootCooldownComponent* shootCooldownComponent = aimingEntity->GetComponent<ShootCooldownComponent>();

    // There can be no cooldown component if aimingEntity is from another player since this component is private
    // Ignore this since on client this function just draws a ray for debugging purposes. On server it will work correctly
    if (shootCooldownComponent != nullptr)
    {
        uint32 lastShootFrameId = shootCooldownComponent->GetLastShootFrameId();
        if (clientFrameId - lastShootFrameId < SHOOTER_SHOOT_COOLDOWN_FRAMES)
        {
            return;
        }

        shootCooldownComponent->SetLastShootFrameId(clientFrameId);
    }

    ShooterAimComponent* aimComponent = aimingEntity->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent != nullptr);

    Vector3 aimRayOrigin;
    Vector3 aimRayDirection;
    Vector3 aimRayEnd;
    Entity* aimRayEndEntity;
    GetCurrentAimRay(*aimComponent, RaycastFilter::IGNORE_SOURCE, aimRayOrigin, aimRayDirection, aimRayEnd, &aimRayEndEntity);

    Entity* weaponBarrelEntity = aimingEntity->FindByName(SHOOTER_GUN_BARREL_ENTITY_NAME);
    if (weaponBarrelEntity != nullptr)
    {
        Vector3 shootStart = weaponBarrelEntity->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslationVector();
        Vector3 shootDirection = aimRayEnd - shootStart;
        shootDirection.Normalize();

        aimingEntity->GetComponent<ShooterStateComponent>()->raycastAttackFrameId = clientFrameId;

        if (IsServer(GetScene()))
        {
            PhysicsSystem* physics = GetScene()->GetSystem<PhysicsSystem>();
            DVASSERT(physics != nullptr);

            NetworkReplicationComponent* replicationComponent = aimingEntity->GetComponent<NetworkReplicationComponent>();
            DVASSERT(replicationComponent != nullptr);

            const FastName& token = GetScene()->GetSingletonComponent<NetworkGameModeSingleComponent>()->GetToken(replicationComponent->GetNetworkPlayerID());
            DVASSERT(token.IsValid());

            NetworkTimeSingleComponent* timeSingleComponent = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
            DVASSERT(timeSingleComponent != nullptr);

            int32 fdiff = timeSingleComponent->GetClientServerDiff(token);
            if (fdiff < 0)
            {
                Logger::Error("fdiff is negative");
                return;
            }

            uint32 numFramesToRollBack = std::min(10, fdiff);
            const uint32 frameId = timeSingleComponent->GetFrameId();
            uint32 pastFrameId = clientFrameId == 0 ? frameId : clientFrameId - numFramesToRollBack;

            physx::PxRaycastHit hit;
            CharacterMirrorsSingleComponent* mirrorsSingleComponent = GetScene()->GetSingletonComponent<CharacterMirrorsSingleComponent>();
            QueryFilterCallback filterCallback(mirrorsSingleComponent->GetMirrorForCharacter(aimingEntity), RaycastFilter::IGNORE_SOURCE);
            ComponentMask possibleComponents = ComponentUtils::MakeMask<CapsuleCharacterControllerComponent>() | ComponentUtils::MakeMask<DynamicBodyComponent>();
            bool collision = NetworkPhysicsUtils::GetRaycastHitInPast(*aimingEntity->GetScene(), possibleComponents, shootStart, shootDirection, SHOOTER_MAX_SHOOTING_DISTANCE, pastFrameId, &filterCallback, hit);
            if (collision)
            {
                Component* component = static_cast<Component*>(hit.actor->userData);

                if (component->GetType() == Type::Instance<DynamicBodyComponent>())
                {
                    Entity* player = mirrorsSingleComponent->GetCharacterFromMirror(component->GetEntity());
                    if (player != nullptr)
                    {
                        HealthComponent* healthComponent = player->GetComponent<HealthComponent>();
                        DVASSERT(healthComponent);

                        healthComponent->DecHealth(1, frameId);
                    }
                }
            }
        }
    }
}

void ShooterPlayerAttackSystem::RocketAttack(DAVA::Entity* aimingEntity, DAVA::uint32 clientFrameId) const
{
    using namespace DAVA;

    DVASSERT(aimingEntity != nullptr);

    ShootCooldownComponent* shootCooldownComponent = aimingEntity->GetComponent<ShootCooldownComponent>();

    // There can be no cooldown component if aimingEntity is from another player since this component is private
    // Ignore this since on client this function just draws a ray for debugging purposes. On server it will work correctly
    if (shootCooldownComponent != nullptr)
    {
        uint32 lastShootFrameId = shootCooldownComponent->GetLastShootFrameId();
        if (clientFrameId - lastShootFrameId < SHOOTER_SHOOT_COOLDOWN_FRAMES)
        {
            return;
        }

        shootCooldownComponent->SetLastShootFrameId(clientFrameId);
    }

    ShooterAimComponent* aimComponent = aimingEntity->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent != nullptr);

    Entity* weaponBarrelEntity = aimingEntity->FindByName(SHOOTER_GUN_BARREL_ENTITY_NAME);
    if (weaponBarrelEntity != nullptr)
    {
        const NetworkReplicationComponent* shooterReplComp = aimingEntity->GetComponent<NetworkReplicationComponent>();
        NetworkPlayerID playerID = shooterReplComp->GetNetworkPlayerID();
        FrameActionID shootActionId(clientFrameId, playerID, 1);
        NetworkID entityId = NetworkIdSystem::GetEntityIdFromAction(shootActionId);
        Entity* rocket = GetScene()->GetSingletonComponent<NetworkEntitiesSingleComponent>()->FindByID(entityId);
        if (!rocket)
        {
            rocket = new Entity;
            rocket->AddComponent(new NetworkTransformComponent());

            NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent();
            networkPredictComponent->SetFrameActionID(shootActionId);
            networkPredictComponent->AddPredictedComponent(Type::Instance<ShooterRocketComponent>());
            networkPredictComponent->AddPredictedComponent(Type::Instance<NetworkTransformComponent>());
            rocket->AddComponent(networkPredictComponent);

            ShooterRocketComponent* rocketComp = new ShooterRocketComponent();
            rocketComp->shooterId = shooterReplComp->GetNetworkID();
            rocket->AddComponent(rocketComp);

            NetworkReplicationComponent* bulletReplComp = new NetworkReplicationComponent();
            bulletReplComp->SetNetworkPlayerID(playerID);
            rocket->AddComponent(bulletReplComp);

            const Matrix4& weaponTrans = weaponBarrelEntity->GetComponent<TransformComponent>()->GetWorldTransform();
            rocket->GetComponent<TransformComponent>()->SetLocalTransform(weaponTrans.GetTranslationVector(),
                                                                          weaponTrans.GetRotation(),
                                                                          weaponTrans.GetScaleVector());
            GetScene()->AddNode(rocket);
        }
    }
}
