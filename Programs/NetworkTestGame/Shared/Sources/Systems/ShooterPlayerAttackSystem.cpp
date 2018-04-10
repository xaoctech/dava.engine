#include "Systems/ShooterPlayerAttackSystem.h"
#include "Components/ShooterRoleComponent.h"
#include "Components/ShooterAimComponent.h"
#include "Components/HealthComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Components/ShooterRocketComponent.h"
#include "Components/ShooterStateComponent.h"
#include "Components/RocketSpawnComponent.h"
#include "Components/SingleComponents/EffectQueueSingleComponent.h"
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
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h>
#include <NetworkCore/Scene3D/Systems/NetworkIdSystem.h>
#include <NetworkCore/LagCompensatedAction.h>
#include <NetworkPhysics/NetworkPhysicsUtils.h>
#include <NetworkPhysics/CharacterMirrorsSingleComponent.h>
#include <Physics/PhysicsSystem.h>
#include <Physics/Core/CollisionShapeComponent.h>
#include <Physics/Controllers/CapsuleCharacterControllerComponent.h>
#include <Physics/Core/Private/PhysicsMath.h>
#include <Physics/Core/DynamicBodyComponent.h>
#include <Debug/ProfilerCPU.h>

#include <physx/PxRigidActor.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterPlayerAttackSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterPlayerAttackSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterPlayerAttackSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 14.0f)]
    .End();
}

// Invokes lag compensated raycast attack
class LagCompensatedRaycastAttack : public DAVA::LagCompensatedAction
{
public:
    LagCompensatedRaycastAttack(DAVA::Entity* player, const DAVA::Vector3& origin, const DAVA::Vector3& direction)
        : player(player)
        , origin(origin)
        , direction(direction)
    {
    }

    const physx::PxRaycastHit* GetHitInfo()
    {
        if (collision)
        {
            return &hit;
        }

        return nullptr;
    }

private:
    const DAVA::ComponentMask* GetLagCompensatedComponentsForEntity(DAVA::Entity* e) override
    {
        using namespace DAVA;

        DVASSERT(e != nullptr);

        if (e != player)
        {
            // Only rewind entities that have dynamic body or cct components
            const CapsuleCharacterControllerComponent* cctComponent = e->GetComponent<CapsuleCharacterControllerComponent>();
            const DynamicBodyComponent* dynamicBodyComponent = e->GetComponent<DynamicBodyComponent>();
            if (cctComponent != nullptr || dynamicBodyComponent != nullptr)
            {
                static const ComponentMask lagCompensatedComponentsMask = ComponentUtils::MakeMask<NetworkTransformComponent>();
                return &lagCompensatedComponentsMask;
            }
        }

        return nullptr;
    }

    void Action(DAVA::Scene& scene) override
    {
        // Invoke raycast
        // All CCTs should already been rewound if we're on a server
        QueryFilterCallback filterCallback(player, RaycastFilter::IGNORE_CONTROLLER | RaycastFilter::IGNORE_SOURCE);
        collision = GetRaycastHit(*(player->GetScene()), origin, direction, SHOOTER_MAX_SHOOTING_DISTANCE, &filterCallback, hit);
    }

    void OnComponentsInPast(DAVA::Scene& scene, const DAVA::Vector<DAVA::Entity*>& entities) override
    {
        UpdatePhysicsEntities(scene);
    }

    void OnComponentsInPresent(DAVA::Scene& scene, const DAVA::Vector<DAVA::Entity*>& entities) override
    {
        UpdatePhysicsEntities(scene);
    }

    void UpdatePhysicsEntities(DAVA::Scene& scene)
    {
        using namespace DAVA;

        PhysicsSystem* physicsSystem = scene.GetSystem<PhysicsSystem>();
        DVASSERT(physicsSystem != nullptr);

        // Sync transforms so that raycast uses new positions
        physicsSystem->SyncTransformToPhysx();
    }

private:
    DAVA::Entity* player;
    const DAVA::Vector3 origin;
    const DAVA::Vector3 direction;

    bool collision = false;
    physx::PxRaycastHit hit;
};

ShooterPlayerAttackSystem::ShooterPlayerAttackSystem(DAVA::Scene* scene)
    : DAVA::BaseSimulationSystem(scene, DAVA::ComponentUtils::MakeMask<ShooterRoleComponent, ShooterStateComponent>())
{
    using namespace DAVA;

    // TODO: get rid of these identifiers
    Mouse* kb = GetEngineContext()->deviceManager->GetMouse();
    uint32 mouseId = ~0;
    if (kb != nullptr)
    {
        mouseId = kb->GetId();
    }

    ActionsSingleComponent* actionsSingleComponent = scene->GetSingleComponent<ActionsSingleComponent>();
    actionsSingleComponent->AddAvailableDigitalAction(SHOOTER_ACTION_ATTACK_BULLET);
    actionsSingleComponent->AddAvailableDigitalAction(SHOOTER_ACTION_INTERACT);
    actionsSingleComponent->AddAvailableDigitalAction(SHOOTER_ACTION_ATTACK_ROCKET);
    actionsSingleComponent->AddAvailableDigitalAction(SHOOTER_ACTION_ATTACK_ROCKET2);

    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_ATTACK_BULLET, eInputElements::MOUSE_LBUTTON, mouseId, DigitalElementState::JustPressed());
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_ATTACK_ROCKET, eInputElements::MOUSE_RBUTTON, mouseId, DigitalElementState::Pressed());
    actionsSingleComponent->CollectDigitalAction(SHOOTER_ACTION_ATTACK_ROCKET2, eInputElements::MOUSE_MBUTTON, mouseId, DigitalElementState::JustPressed());

    optionsComp = scene->GetSingleComponent<BattleOptionsSingleComponent>();
    effectQueue = scene->GetSingleComponent<EffectQueueSingleComponent>();

    entityGroup = scene->AquireEntityGroup<ShooterRoleComponent, ShooterStateComponent>();
}

void ShooterPlayerAttackSystem::ProcessFixed(DAVA::float32 dt)
{
    using namespace DAVA;

    DAVA_PROFILER_CPU_SCOPE("ShooterPlayerAttackSystem::ProcessFixed");

    for (Entity* entity : entityGroup->GetEntities())
    {
        // TODO: separate components for each role
        if (entity->GetComponent<ShooterRoleComponent>()->GetRole() != ShooterRoleComponent::Role::Player)
        {
            continue;
        }

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

void ShooterPlayerAttackSystem::ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration)
{
    using namespace DAVA;

    ShooterRoleComponent* roleComponent = entity->GetComponent<ShooterRoleComponent>();
    DVASSERT(roleComponent != nullptr);
    DVASSERT(roleComponent->GetRole() == ShooterRoleComponent::Role::Player);

    bool hasActionAttackRocket = false;
    bool hasActionAttackRocket2 = false;

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
        hasActionAttackRocket2 |= (action == SHOOTER_ACTION_ATTACK_ROCKET2);
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
                RocketAttack(entity, clientFrameId, true);
                rocketSpawnProgress = 0;
            }
        }
        else
        {
            rocketSpawnProgress = 0;
        }

        if (hasActionAttackRocket2)
        {
            RocketAttack(entity, clientFrameId, false);
        }
    }
}

void ShooterPlayerAttackSystem::ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration)
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

    NetworkID bulletId = NetworkID::CreatePlayerActionId(playerId, clientFrameId, 0);

    NetworkReplicationComponent* bulletNetworkReplicationComponent = new NetworkReplicationComponent(bulletId);
    bullet->AddComponent(bulletNetworkReplicationComponent);

    ComponentMask predictedComponents;
    predictedComponents.Set<NetworkTransformComponent>();
    NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent(predictedComponents);
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

    // There can be no cooldown bodyComponent if aimingEntity is from another player since this bodyComponent is private
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
    GetCurrentAimRay(*aimComponent, RaycastFilter::IGNORE_SOURCE | RaycastFilter::IGNORE_CONTROLLER, aimRayOrigin, aimRayDirection,
                     aimRayEnd, &aimRayEndEntity);

    Entity* weaponBarrelEntity = aimingEntity->FindByName(SHOOTER_GUN_BARREL_ENTITY_NAME);
    if (weaponBarrelEntity != nullptr)
    {
        Vector3 shootStart = weaponBarrelEntity->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslationVector();
        Vector3 shootDirection = aimRayEnd - shootStart;
        shootDirection.Normalize();

        aimingEntity->GetComponent<ShooterStateComponent>()->raycastAttackFrameId = clientFrameId;

        NetworkReplicationComponent* replicationComponent = aimingEntity->GetComponent<NetworkReplicationComponent>();
        DVASSERT(replicationComponent != nullptr);
        NetworkPlayerID playerId = replicationComponent->GetNetworkPlayerID();

        LagCompensatedRaycastAttack raycastAttack(aimingEntity, shootStart, shootDirection);

        if (optionsComp->isEnemyRewound)
        {
            raycastAttack.Invoke(*GetScene(), playerId, clientFrameId);
        }
        else
        {
            raycastAttack.InvokeWithoutLagCompensation(*GetScene());
        }

        if (IsServer(GetScene()))
        {
            const physx::PxRaycastHit* hit = raycastAttack.GetHitInfo();
            if (hit != nullptr)
            {
                Component* bodyComponent = static_cast<Component*>(hit->actor->userData);
                if (bodyComponent->GetType()->Is<DynamicBodyComponent>())
                {
                    Entity* entity = bodyComponent->GetEntity();
                    DVASSERT(entity);

                    HealthComponent* healthComponent = entity->GetComponent<HealthComponent>();
                    if (healthComponent)
                    {
                        CollisionShapeComponent* shapeComponent = CollisionShapeComponent::GetComponent(hit->shape);
                        if (shapeComponent->GetJointName().IsValid() && shapeComponent->GetJointName().size() > 0)
                        {
                            uint32 damage = GetBodyPartDamage(shapeComponent->GetJointName());
                            healthComponent->DecHealth(damage, clientFrameId);
                        }
                    }
                }
            }
        }
    }
}

void ShooterPlayerAttackSystem::RocketAttack(DAVA::Entity* aimingEntity, DAVA::uint32 clientFrameId, bool multirocket) const
{
    using namespace DAVA;

    DVASSERT(aimingEntity != nullptr);

    ShootCooldownComponent* shootCooldownComponent = aimingEntity->GetComponent<ShootCooldownComponent>();

    // There can be no cooldown bodyComponent if aimingEntity is from another player since this bodyComponent is private
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
        NetworkID entityId = NetworkID::CreatePlayerActionId(playerID, clientFrameId, 1);
        Entity* rocket = GetScene()->GetSingleComponent<NetworkEntitiesSingleComponent>()->FindByID(entityId);
        if (!rocket)
        {
            rocket = new Entity;
            rocket->AddComponent(new NetworkTransformComponent());

            ComponentMask predictedComponentsMask;
            predictedComponentsMask.Set<ShooterRocketComponent>();
            predictedComponentsMask.Set<NetworkTransformComponent>();

            NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent(predictedComponentsMask);
            rocket->AddComponent(networkPredictComponent);

            ShooterRocketComponent* rocketComp = new ShooterRocketComponent();
            rocketComp->shooterId = shooterReplComp->GetNetworkID();
            rocketComp->multirocket = multirocket;
            rocket->AddComponent(rocketComp);

            NetworkReplicationComponent* bulletReplComp = new NetworkReplicationComponent(entityId);
            bulletReplComp->SetForReplication<NetworkTransformComponent>(M::Privacy::PUBLIC);
            bulletReplComp->SetForReplication<ShooterRocketComponent>(M::Privacy::PUBLIC);
            rocket->AddComponent(bulletReplComp);

            const Matrix4& weaponTrans = weaponBarrelEntity->GetComponent<TransformComponent>()->GetWorldTransform();
            rocket->GetComponent<TransformComponent>()->SetLocalTransform(weaponTrans.GetTranslationVector(),
                                                                          weaponTrans.GetRotation(),
                                                                          weaponTrans.GetScaleVector());

            // effect: rocket shot
            NetworkID shotEffectId = NetworkID::CreatePlayerActionId(playerID, clientFrameId, 2);
            effectQueue->CreateEffect(1)
            .SetDuration(5.f)
            .SetPosition(weaponTrans.GetTranslationVector())
            .SetRotation(weaponTrans.GetRotation())
            .SetNetworkId(shotEffectId);

            // effect: rocket smoke track
            NetworkID trackEffectId = NetworkID::CreatePlayerActionId(playerID, clientFrameId, 3);
            effectQueue->CreateEffect(2)
            .SetNetworkId(trackEffectId)
            .SetParentId(entityId);

            GetScene()->AddNode(rocket);
        }
    }
}
