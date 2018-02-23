#include "Systems/ShooterEntityFillSystem.h"
#include "ShooterConstants.h"
#include "ShooterUtils.h"
#include "Components/ShooterRoleComponent.h"
#include "Components/ShooterAimComponent.h"
#include "Components/ShooterProjectileComponent.h"
#include "Components/ShooterCarUserComponent.h"
#include "Components/HealthComponent.h"
#include "Components/GameStunnableComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Components/ShooterMirroredCharacterComponent.h"
#include "Components/RocketSpawnComponent.h"
#include "Components/ShooterStateComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/TransformInterpolationComponent.h>
#include <Scene3D/Components/MotionComponent.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Entity/ComponentUtils.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkRemoteInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkDebugDrawComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>

#include <Physics/CapsuleCharacterControllerComponent.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/BoxShapeComponent.h>
#include <Physics/PhysicsSystem.h>
#include <Physics/PhysicsUtils.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/VehicleCarComponent.h>
#include <Physics/VehicleWheelComponent.h>
#include <Physics/VehicleChassisComponent.h>
#include <Physics/BoxShapeComponent.h>
#include <Physics/CapsuleShapeComponent.h>
#include <Physics/MeshShapeComponent.h>

#include <NetworkPhysics/NetworkDynamicBodyComponent.h>
#include <NetworkPhysics/CharacterMirrorsSingleComponent.h>

#include <algorithm>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterEntityFillSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterEntityFillSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterEntityFillSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 10.0f)]
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

    CapsuleCharacterControllerComponent* controllerComponent = new CapsuleCharacterControllerComponent();
    controllerComponent->SetHeight(SHOOTER_CHARACTER_CAPSULE_HEIGHT);
    controllerComponent->SetRadius(SHOOTER_CHARACTER_CAPSULE_RADIUS);
    controllerComponent->SetTypeMask(SHOOTER_CHARACTER_COLLISION_TYPE);
    controllerComponent->SetTypeMaskToCollideWith(UINT32_MAX);
    entity->AddComponent(controllerComponent);

    if (IsServer(GetScene()))
    {
        NetworkTransformComponent* networkTransformComponent = new NetworkTransformComponent();
        entity->AddComponent(networkTransformComponent);

        entity->AddComponent(new NetworkInputComponent());

        NetworkRemoteInputComponent* remoteInputComponent = new NetworkRemoteInputComponent();
        remoteInputComponent->AddActionToReplicate(SHOOTER_ACTION_MOVE_FORWARD);
        remoteInputComponent->AddActionToReplicate(SHOOTER_ACTION_MOVE_BACKWARD);
        remoteInputComponent->AddActionToReplicate(SHOOTER_ACTION_MOVE_LEFT);
        remoteInputComponent->AddActionToReplicate(SHOOTER_ACTION_MOVE_RIGHT);
        remoteInputComponent->AddActionToReplicate(SHOOTER_ACTION_ACCELERATE);
        remoteInputComponent->AddActionToReplicate(SHOOTER_ACTION_ATTACK_BULLET);
        entity->AddComponent(remoteInputComponent);

        ShooterAimComponent* aimComponent = new ShooterAimComponent();
        entity->AddComponent(aimComponent);

        entity->AddComponent(new ShooterCarUserComponent());

        HealthComponent* healthComponent = new HealthComponent();
        healthComponent->SetHealth(SHOOTER_CHARACTER_MAX_HEALTH);
        entity->AddComponent(healthComponent);

        entity->AddComponent(new NetworkDynamicBodyComponent());

        entity->AddComponent(new GameStunnableComponent());
        entity->AddComponent(new ShootCooldownComponent());
        entity->AddComponent(new RocketSpawnComponent());
        entity->AddComponent(new ShooterStateComponent());

        // Mirror

        entity->AddComponent(new ShooterMirroredCharacterComponent());

        Entity* mirror = PhysicsUtils::CreateCharacterMirror(controllerComponent);
        DVASSERT(mirror != nullptr);

        DynamicBodyComponent* mirrorBodyComponent = mirror->GetComponent<DynamicBodyComponent>();
        DVASSERT(mirrorBodyComponent != nullptr);

        mirrorBodyComponent->SetLinearDamping(0.5f);
        mirrorBodyComponent->SetLockFlags(static_cast<DynamicBodyComponent::eLockFlags>(DynamicBodyComponent::eLockFlags::AngularX | DynamicBodyComponent::eLockFlags::AngularY | DynamicBodyComponent::eLockFlags::AngularZ));

        CapsuleShapeComponent* mirrorShapeComponent = mirror->GetComponent<CapsuleShapeComponent>();
        DVASSERT(mirrorShapeComponent != nullptr);

        mirrorShapeComponent->SetOverrideMass(true);
        mirrorShapeComponent->SetMass(120.0f);

        GetScene()->AddNode(mirror);
        GetScene()->GetSingletonComponent<CharacterMirrorsSingleComponent>()->AddMirrorForCharacter(entity, mirror);
    }

    if (!IsServer(GetScene()))
    {
        if (IsClientOwner(GetScene(), entity))
        {
            NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent();
            networkPredictComponent->AddPredictedComponent(Type::Instance<NetworkTransformComponent>());
            networkPredictComponent->AddPredictedComponent(Type::Instance<ShooterAimComponent>());
            networkPredictComponent->AddPredictedComponent(Type::Instance<RocketSpawnComponent>());
            entity->AddComponent(networkPredictComponent);

            name = "My player";
        }
        else
        {
            // No need for gravity for replicated CCTs
            controllerComponent->SetMovementMode(CharacterControllerComponent::MovementMode::Flying);
            name = "Enemy player";

            TransformInterpolationComponent* tic = new TransformInterpolationComponent();
            tic->time = 2.0f;
            entity->AddComponent(tic);
        }
    }

    entity->SetName(name.c_str());

    // Create character

    ScopedPtr<Scene> characterScene(new Scene());
    characterScene->LoadScene("~res:/TestCharacterControllerModule/character/character_mesh.sc2");

    Entity* characterSourceEntity = characterScene->FindByName("character");
    DVASSERT(characterSourceEntity != nullptr);

    MotionComponent* motionComponent = new MotionComponent();
    motionComponent->SetDescriptorPath("~res:/TestCharacterControllerModule/character/character_motion.yaml");

    ScopedPtr<Entity> characterEntity(characterSourceEntity->Clone());
    characterEntity->SetName("Character");
    characterEntity->AddComponent(motionComponent);
    entity->AddNode(characterEntity);

    ScopedPtr<Scene> weaponScene(new Scene());
    weaponScene->LoadScene("~res:/TestCharacterControllerModule/character/weapon_mesh.sc2");
    Entity* weaponSourceEntity = weaponScene->FindByName("weapon");
    DVASSERT(weaponSourceEntity != nullptr);

    ScopedPtr<Entity> weaponEntity(weaponSourceEntity->Clone());
    weaponEntity->SetName("Weapon");
    characterEntity->AddNode(weaponEntity);

    if (IsClient(this))
    {
        NetworkDebugDrawComponent* debugDrawComponent = new NetworkDebugDrawComponent();
        debugDrawComponent->box = characterEntity->GetWTMaximumBoundingBoxSlow();
        entity->AddComponent(debugDrawComponent);
    }
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

    Entity* chassis = carModelScene->GetEntityByID(6)->Clone();

    Entity* gun = carModelScene->GetEntityByID(7)->Clone();
    entity->AddNode(gun);

    if (IsServer(this))
    {
        TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
        NetworkTransformComponent* networkTransform = new NetworkTransformComponent();
        networkTransform->SetPosition(transformComponent->GetPosition());
        networkTransform->SetOrientation(transformComponent->GetRotation());
        entity->AddComponent(networkTransform);

        VehicleCarComponent* carComponent = new VehicleCarComponent();
        entity->AddComponent(carComponent);

        DynamicBodyComponent* carDynamicBody = new DynamicBodyComponent();
        entity->AddComponent(carDynamicBody);

        transformComponent = chassis->GetComponent<TransformComponent>();
        networkTransform = new NetworkTransformComponent();
        networkTransform->SetPosition(transformComponent->GetPosition());
        networkTransform->SetOrientation(transformComponent->GetRotation());
        chassis->AddComponent(networkTransform);

        NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent();
        chassis->AddComponent(replicationComponent);

        entity->AddNode(chassis);

        uint32 wheelIds[4]{ 3, 2, 4, 5 };
        for (uint32 i = 0; i < COUNT_OF(wheelIds); ++i)
        {
            Entity* wheel = carModelScene->GetEntityByID(wheelIds[i])->Clone();

            transformComponent = wheel->GetComponent<TransformComponent>();
            networkTransform = new NetworkTransformComponent();
            networkTransform->SetPosition(transformComponent->GetPosition());
            networkTransform->SetOrientation(transformComponent->GetRotation());
            wheel->AddComponent(networkTransform);

            NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent();
            wheel->AddComponent(replicationComponent);

            entity->AddNode(wheel);
        }
    }
    else
    {
        Entity* wheel = carModelScene->GetEntityByID(3)->Clone();
        DVASSERT(entity->GetChildrenCount() == 6);

        for (int32 i = 0; i < entity->GetChildrenCount(); ++i)
        {
            Entity* child = entity->GetChild(i);
            child->RemoveComponent<VehicleChassisComponent>();

            if (child != gun)
            {
                // Chassis first, followed by the wheels
                // Guaranteed by replication order
                if (i == 0)
                {
                    child->AddComponent(chassis->GetComponent<RenderComponent>()->Clone(child));
                    child->AddComponent(chassis->GetComponent<BoxShapeComponent>()->Clone(child));
                }
                else
                {
                    child->AddComponent(wheel->GetComponent<RenderComponent>()->Clone(child));
                }
            }
        }

        DynamicBodyComponent* carDynamicBody = new DynamicBodyComponent();
        carDynamicBody->SetIsKinematic(true);
        entity->AddComponent(carDynamicBody);
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
