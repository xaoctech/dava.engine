#include "Systems/ShooterEntityFillSystem.h"

#include "ShooterConstants.h"
#include "ShooterUtils.h"

#include "Components/ShooterRoleComponent.h"
#include "Components/ShooterAimComponent.h"
#include "Components/ShooterProjectileComponent.h"
#include "Components/ShooterCarUserComponent.h"
#include "Components/ExternalImpulseComponent.h"
#include "Components/HealthComponent.h"
#include "Components/GameStunnableComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Components/RocketSpawnComponent.h"
#include "Components/ShooterStateComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "Visibility/ObservableComponent.h"
#include "Visibility/SimpleVisibilityShapeComponent.h"

#include "Visibility/CharacterVisibilityShapeComponent.h"
#include "Visibility/ObservableComponent.h"

#include "Bots/BattleRoyaleBehaviorComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/MotionComponent.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/SkeletonComponent.h>
#include <Entity/ComponentUtils.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkRemoteInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkDebugDrawComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkMovementComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkMotionComponent.h>

#include <NetworkPhysics/HitboxesDebugDrawComponent.h>

#include <Physics/Controllers/CapsuleCharacterControllerComponent.h>
#include <Physics/Core/DynamicBodyComponent.h>
#include <Physics/Core/BoxShapeComponent.h>
#include <Physics/PhysicsSystem.h>
#include <Physics/Core/PhysicsUtils.h>
#include <Physics/Core/DynamicBodyComponent.h>
#include <Physics/Vehicles/VehicleCarComponent.h>
#include <Physics/Vehicles/VehicleWheelComponent.h>
#include <Physics/Vehicles/VehicleChassisComponent.h>
#include <Physics/Core/BoxShapeComponent.h>
#include <Physics/Core/ConvexHullShapeComponent.h>
#include <Physics/Core/CapsuleShapeComponent.h>
#include <Physics/Core/MeshShapeComponent.h>

#include <algorithm>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterEntityFillSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterEntityFillSystem>::Begin()[M::SystemTags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterEntityFillSystem::ProcessFixed)[M::SystemProcessInfo(SPI::Group::Gameplay, SPI::Type::Fixed, 10.0f)]
    .End();
}

ShooterEntityFillSystem::ShooterEntityFillSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene, DAVA::ComponentUtils::MakeMask<ShooterRoleComponent>())
{
}

void ShooterEntityFillSystem::AddEntity(DAVA::Entity* entity)
{
    ShooterRoleComponent* roleComponent = entity->GetComponent<ShooterRoleComponent>();
    DVASSERT(roleComponent != nullptr);

    DVASSERT(std::find(newRoles.begin(), newRoles.end(), roleComponent) == newRoles.end());
    newRoles.push_back(roleComponent);
}

void ShooterEntityFillSystem::RemoveEntity(DAVA::Entity* entity)
{
    ShooterRoleComponent* roleComponent = entity->GetComponent<ShooterRoleComponent>();
    DVASSERT(roleComponent != nullptr);

    newRoles.erase(std::remove(newRoles.begin(), newRoles.end(), roleComponent), newRoles.end());
}

void ShooterEntityFillSystem::ProcessFixed(DAVA::float32 dt)
{
    for (ShooterRoleComponent* roleComponent : newRoles)
    {
        ShooterRoleComponent::Role role = roleComponent->GetRole();

        if (role == ShooterRoleComponent::Role::Player)
        {
            FillPlayerEntity(roleComponent->GetEntity());
        }
        else if (role == ShooterRoleComponent::Role::Car)
        {
            FillCarEntity(roleComponent->GetEntity());
        }
        else if (role == ShooterRoleComponent::Role::Bullet)
        {
            FillBulletEntity(roleComponent->GetEntity());
        }
    }

    newRoles.clear();
}

void ShooterEntityFillSystem::PrepareForRemove()
{
    newRoles.clear();
}

void ShooterEntityFillSystem::FillPlayerEntity(DAVA::Entity* entity)
{
    using namespace DAVA;

    DVASSERT(entity != nullptr);

    String name = "Player";

    const bool isServer = IsServer(GetScene());
    const bool isClientOwner = IsClientOwner(entity);

    if (isServer || isClientOwner)
    {
        const BattleOptionsSingleComponent* optionsComponent = GetScene()->GetSingleComponentForRead<BattleOptionsSingleComponent>(this);

        CapsuleCharacterControllerComponent* controllerComponent = new CapsuleCharacterControllerComponent();
        controllerComponent->SetHeight(SHOOTER_CHARACTER_CAPSULE_HEIGHT);
        controllerComponent->SetRadius(SHOOTER_CHARACTER_CAPSULE_RADIUS);
        controllerComponent->SetTypeMask(SHOOTER_CHARACTER_COLLISION_TYPE);
        uint32 collisionMask = SHOOTER_CCT_COLLIDE_WITH_MASK;
        if (!optionsComponent->collideCharacters)
        {
            collisionMask &= ~SHOOTER_CHARACTER_COLLISION_TYPE;
        }
        controllerComponent->SetTypeMaskToCollideWith(collisionMask);
        entity->AddComponent(controllerComponent);
    }

    if (isServer)
    {
        ShooterRoleComponent* roleComponent = entity->GetComponent<ShooterRoleComponent>();
        NetworkPlayerID playerId = roleComponent->playerID;

        TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
        const Transform& transform = transformComponent->GetLocalTransform();
        Vector3 position = GetRandomPlayerSpawnPosition();
        transformComponent->SetLocalTransform(Transform(position, transform.GetScale(), transform.GetRotation()));

        ExternalImpulseComponent* impulseComponent = new ExternalImpulseComponent();
        entity->AddComponent(impulseComponent);

        NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent(NetworkID::CreatePlayerOwnId(playerId));

        replicationComponent->SetForReplication<ShooterRoleComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<NetworkPlayerComponent>(M::Privacy::PRIVATE);
        replicationComponent->SetForReplication<NetworkInputComponent>(M::Privacy::PRIVATE);
        replicationComponent->SetForReplication<NetworkTransformComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<NetworkRemoteInputComponent>(M::Privacy::PUBLIC);
        // TODO: NetworkMotionComponent should be send to everyone except owner. New type of privacy required
        // For now it is set as public and predicted to avoid setting values from old frames
        replicationComponent->SetForReplication<NetworkMotionComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<ShooterAimComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<ShooterCarUserComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<HealthComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<GameStunnableComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<ShootCooldownComponent>(M::Privacy::PRIVATE);
        replicationComponent->SetForReplication<RocketSpawnComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<ShooterStateComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<ExternalImpulseComponent>(M::Privacy::PRIVATE);
        replicationComponent->SetForReplication<HitboxesDebugDrawComponent>(M::Privacy::PRIVATE);

        entity->AddComponent(replicationComponent);
        entity->AddComponent(new NetworkPlayerComponent());
        entity->AddComponent(new NetworkInputComponent());
        entity->AddComponent(new NetworkTransformComponent());

        NetworkRemoteInputComponent* remoteInputComponent = new NetworkRemoteInputComponent();
        remoteInputComponent->AddActionToReplicate(SHOOTER_ACTION_MOVE_FORWARD);
        remoteInputComponent->AddActionToReplicate(SHOOTER_ACTION_MOVE_BACKWARD);
        remoteInputComponent->AddActionToReplicate(SHOOTER_ACTION_MOVE_LEFT);
        remoteInputComponent->AddActionToReplicate(SHOOTER_ACTION_MOVE_RIGHT);
        remoteInputComponent->AddActionToReplicate(SHOOTER_ACTION_ACCELERATE);
        remoteInputComponent->AddActionToReplicate(SHOOTER_ACTION_ATTACK_BULLET);
        entity->AddComponent(remoteInputComponent);

        NetworkMotionComponent* animationComponent = new NetworkMotionComponent();
        animationComponent->paramNames.push_back(FastName(MOTION_PARAM_RUNNING));
        animationComponent->paramNames.push_back(FastName(MOTION_PARAM_DIRECTION_X));
        animationComponent->paramNames.push_back(FastName(MOTION_PARAM_DIRECTION_Y));
        entity->AddComponent(animationComponent);

        entity->AddComponent(new HitboxesDebugDrawComponent());

        entity->AddComponent(new ShooterAimComponent());
        entity->AddComponent(new ShooterCarUserComponent());

        HealthComponent* healthComponent = new HealthComponent();
        healthComponent->SetHealth(SHOOTER_CHARACTER_MAX_HEALTH);
        entity->AddComponent(healthComponent);

        entity->AddComponent(new GameStunnableComponent());
        entity->AddComponent(new ShootCooldownComponent());
        entity->AddComponent(new RocketSpawnComponent());
        entity->AddComponent(new ShooterStateComponent());
    }
    else
    {
        NetworkDebugDrawComponent* debugDrawComponent = new NetworkDebugDrawComponent();
        debugDrawComponent->box = entity->GetWTMaximumBoundingBoxSlow();
        entity->AddComponent(debugDrawComponent);

        if (isClientOwner)
        {
            ComponentMask predictionMask;
            predictionMask.Set<NetworkTransformComponent>();
            predictionMask.Set<NetworkMotionComponent>();
            predictionMask.Set<ShooterAimComponent>();
            predictionMask.Set<ExternalImpulseComponent>();
            predictionMask.Set<RocketSpawnComponent>();

            NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent(predictionMask);
            entity->AddComponent(networkPredictComponent);

            name = "My player";
        }

        BattleOptionsSingleComponent* optionsComp = GetScene()->GetSingleComponent<BattleOptionsSingleComponent>();
        if (optionsComp->options.playerKind.GetId() == PlayerKind::Id::SHOOTER_BATTLE_ROYALE_BOT)
        {
            bool isActor = IsClientOwner(this, entity);
            BattleRoyaleBehaviorComponent* behaviorComponent = new BattleRoyaleBehaviorComponent();
            entity->AddComponent(behaviorComponent);
        }

        // entity->AddComponent(new NetworkMovementComponent());
    }

    entity->SetName(name.c_str());

    // Create character

    ScopedPtr<Scene> characterScene(new Scene());
    characterScene->LoadScene("~res:/TestCharacterControllerModule/character/character_mesh.sc2");

    Entity* characterSourceEntity = characterScene->FindByName("character");
    DVASSERT(characterSourceEntity != nullptr);

    MotionComponent* motionComponent = new MotionComponent();
    motionComponent->SetDescriptorPath("~res:/TestCharacterControllerModule/character/character_motion.yaml");

    for (uint32 i = 0; i < characterSourceEntity->GetComponentCount<BoxShapeComponent>(); ++i)
    {
        entity->AddComponent(characterSourceEntity->GetComponent<BoxShapeComponent>(i)->Clone(entity));
    }
    entity->AddComponent(characterSourceEntity->GetComponent<DynamicBodyComponent>()->Clone(entity));
    entity->AddComponent(characterSourceEntity->GetComponent<RenderComponent>()->Clone(entity));
    entity->AddComponent(characterSourceEntity->GetComponent<SkeletonComponent>()->Clone(entity));
    entity->AddComponent(motionComponent);

    entity->AddNode(characterSourceEntity->FindByName("Marker")->Clone());

    ScopedPtr<Scene> weaponScene(new Scene());
    weaponScene->LoadScene("~res:/TestCharacterControllerModule/character/weapon_mesh.sc2");
    Entity* weaponSourceEntity = weaponScene->FindByName("weapon");
    DVASSERT(weaponSourceEntity != nullptr);

    ScopedPtr<Entity> weaponEntity(weaponSourceEntity->Clone());
    weaponEntity->SetName("Weapon");
    entity->AddNode(weaponEntity);
}

void ShooterEntityFillSystem::FillCarEntity(DAVA::Entity* entity)
{
    // TODO: a better way for filling entities on client/server is needed
    // Current way leads to messy code

    using namespace DAVA;

    DVASSERT(entity != nullptr);

    entity->SetName("Car");

    FilePath carModelScenePath("~res:/3d/Jeep/jeepGun-5.sc2");
    ScopedPtr<Scene> carModelScene(new Scene());
    SceneFileV2::eError sceneLoadResult = carModelScene->LoadScene(carModelScenePath);
    DVASSERT(sceneLoadResult == SceneFileV2::ERROR_NO_ERROR);

    // Clone model parts into the entity

    Entity* carReference = carModelScene->GetEntityByID(2);
    uint32 numWheels = carReference->GetComponentCount<VehicleWheelComponent>();

    entity->AddComponent(carReference->GetComponent<RenderComponent>()->Clone(entity));
    entity->AddComponent(carReference->GetComponent<SkeletonComponent>()->Clone(entity));

    if (IsServer(this))
    {
        ShooterRoleComponent* roleComponent = entity->GetComponent<ShooterRoleComponent>();
        NetworkPlayerID playerId = roleComponent->playerID;

        NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent(NetworkID::CreatePlayerOwnId(playerId));
        replicationComponent->SetForReplication<NetworkTransformComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<ShooterRoleComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<DynamicBodyComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<BoxShapeComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<ConvexHullShapeComponent>(M::Privacy::PUBLIC);

        entity->AddComponent(replicationComponent);

        const Transform& transform = entity->GetComponent<TransformComponent>()->GetLocalTransform();
        TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
        NetworkTransformComponent* networkTransform = new NetworkTransformComponent();
        networkTransform->SetPosition(transform.GetTranslation());
        networkTransform->SetOrientation(transform.GetRotation());
        entity->AddComponent(networkTransform);

        entity->AddComponent(carReference->GetComponent<DynamicBodyComponent>()->Clone(entity));
        entity->AddComponent(carReference->GetComponent<VehicleCarComponent>()->Clone(entity));
        entity->AddComponent(carReference->GetComponent<VehicleChassisComponent>()->Clone(entity));
        entity->AddComponent(carReference->GetComponent<BoxShapeComponent>()->Clone(entity));
        for (uint32 i = 0; i < numWheels; ++i)
        {
            entity->AddComponent(carReference->GetComponent<VehicleWheelComponent>(i)->Clone(entity));
            entity->AddComponent(carReference->GetComponent<ConvexHullShapeComponent>(i)->Clone(entity));
        }

        entity->AddComponent(new ObservableComponent());
        entity->AddComponent(new SimpleVisibilityShapeComponent());
    }
    else
    {
        DynamicBodyComponent* carDynamicBody = entity->GetComponent<DynamicBodyComponent>();
        carDynamicBody->SetIsKinematic(true);
    }
}

void ShooterEntityFillSystem::FillBulletEntity(DAVA::Entity* entity)
{
    using namespace DAVA;

    DVASSERT(entity != nullptr);

    DynamicBodyComponent* dynamicBodyComponent = new DynamicBodyComponent();
    dynamicBodyComponent->SetCCDEnabled(true);
    dynamicBodyComponent->SetBodyFlags(PhysicsComponent::eBodyFlags::DISABLE_GRAVITY);
    entity->AddComponent(dynamicBodyComponent);

    BoxShapeComponent* boxShapeComponent = new BoxShapeComponent();
    boxShapeComponent->SetHalfSize(Vector3(0.005f, 0.02f, 0.005f));
    boxShapeComponent->SetTypeMask(SHOOTER_PROJECTILE_COLLISION_TYPE);
    boxShapeComponent->SetTypeMaskToCollideWith(SHOOTER_CHARACTER_COLLISION_TYPE);
    entity->AddComponent(boxShapeComponent);

    ShooterProjectileComponent* projectileComponent = new ShooterProjectileComponent();
    projectileComponent->SetProjectileType(ShooterProjectileComponent::ProjectileType::Bullet);
    entity->AddComponent(projectileComponent);

    NetworkTransformComponent* networkTransformComponent = new NetworkTransformComponent();
    entity->AddComponent(networkTransformComponent);
}