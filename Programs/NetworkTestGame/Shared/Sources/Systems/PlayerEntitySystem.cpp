#include "PlayerEntitySystem.h"

#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/TransformInterpolationComponent.h>
#include <Scene3D/Components/RenderComponent.h>

#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <UI/UIControlSystem.h>

#include <Scene3D/Components/CameraComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkRemoteInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkRemoteInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkDebugDrawComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTrafficLimitComponent.h>

#include "Components/ShootCooldownComponent.h"
#include "Components/PlayerTankComponent.h"
#include "Components/PlayerCarComponent.h"
#include "Components/PlayerCharacterComponent.h"
#include "Components/ShootComponent.h"
#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include "Components/PowerupCatcherComponent.h"
#include "Components/AI/ShooterBehaviorComponent.h"
#include "Components/AI/RandomMovementTaskComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"

#include <Physics/PhysicsSystem.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/VehicleCarComponent.h>
#include <Physics/VehicleWheelComponent.h>
#include <Physics/VehicleChassisComponent.h>
#include <Physics/BoxShapeComponent.h>
#include <Physics/MeshShapeComponent.h>
#include <Physics/BoxCharacterControllerComponent.h>
#include <Physics/CapsuleCharacterControllerComponent.h>
#include <Physics/PhysicsUtils.h>

#include <NetworkPhysics/NetworkDynamicBodyComponent.h>
#include <NetworkPhysics/NetworkVehicleCarComponent.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <Components/ShooterStateComponent.h>

#include "Systems/GameModeSystemCars.h"

using namespace DAVA;

namespace PlayerEntitySystemDetail
{
static const Vector<FastName> movingActions = { FastName("UP"), FastName("DOWN"), FastName("LEFT"), FastName("RIGHT") };
void CollectRemoteMovingInput(NetworkRemoteInputComponent* networkRemoteInputComponent)
{
    for (const FastName& action : movingActions)
    {
        networkRemoteInputComponent->AddActionToReplicate(action);
    }
}
}

DAVA_VIRTUAL_REFLECTION_IMPL(PlayerEntitySystem)
{
    ReflectionRegistrator<PlayerEntitySystem>::Begin()[M::Tags("playerentity")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &PlayerEntitySystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::NORMAL, 1.0f)]
    .End();
}

PlayerEntitySystem::PlayerEntitySystem(DAVA::Scene* scene)
    : SceneSystem(scene, 0)
{
    optionsComp = scene->GetSingletonComponent<BattleOptionsSingleComponent>();

    tanksSubscriber.reset(new EntityGroupOnAdd(GetScene()->AquireEntityGroup<PlayerTankComponent>()));
    carsSubscriber.reset(new EntityGroupOnAdd(GetScene()->AquireEntityGroup<PlayerCarComponent>()));
    charsSubscriber.reset(new EntityGroupOnAdd(GetScene()->AquireEntityGroup<PlayerCharacterComponent>()));
}

void PlayerEntitySystem::Process(DAVA::float32 timeElapsed)
{
    for (Entity* e : tanksSubscriber->entities)
    {
        FillTankPlayerEntity(e);
    }
    tanksSubscriber->entities.clear();

    for (Entity* e : carsSubscriber->entities)
    {
        FillCarPlayerEntity(e);
    }
    carsSubscriber->entities.clear();

    for (Entity* e : charsSubscriber->entities)
    {
        FillCharacterPlayerEntity(e);
    }
    charsSubscriber->entities.clear();
}

void PlayerEntitySystem::FillTankPlayerEntity(DAVA::Entity* entity)
{
    String filePath("~res:/Sniper_2.sc2");
    entity->SetName("Tank");

    if (IsServer(this))
    {
        Logger::Debug("[PlayerEntitySystem::Process] %d vehicle factory", entity->GetID());
        entity->AddComponent(new NetworkInputComponent());

        NetworkRemoteInputComponent* netRemoteInputComp = new NetworkRemoteInputComponent();
        if (optionsComp->isEnemyPredicted)
        {
            PlayerEntitySystemDetail::CollectRemoteMovingInput(netRemoteInputComp);
        }
        entity->AddComponent(netRemoteInputComp);

        entity->AddComponent(new ShootCooldownComponent());
        entity->AddComponent(new NetworkTransformComponent());
        entity->AddComponent(new NetworkPlayerComponent());
        entity->AddComponent(new GameStunnableComponent());
        entity->AddComponent(new HealthComponent());
        entity->AddComponent(new PowerupCatcherComponent());
        entity->AddComponent(new NetworkTrafficLimitComponent());
    }
    else
    {
        const bool isClientOwner = IsClientOwner(this, entity);
        if (isClientOwner)
        {
            NetworkReplicationComponent* netReplComp = entity->GetComponent<NetworkReplicationComponent>();
            Logger::Debug("[PlayerEntitySystem::Process] Set player:%d vehicle:%d", netReplComp->GetNetworkPlayerID(), entity->GetID());
            entity->SetName("MyTank");
            filePath = "~res:/Sniper_1.sc2";

            Camera* camera = new Camera();
            camera->SetUp(Vector3(0.f, 0.f, 1.f));
            camera->SetPosition(Vector3(0.f, -50.f, 100.f));
            camera->SetTarget(Vector3(0.f, 0.f, 0.f));
            camera->RebuildCameraFromValues();
            VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
            const Size2i& physicalSize = vcs->GetPhysicalScreenSize();
            float32 screenAspect = static_cast<float32>(physicalSize.dx) / static_cast<float32>(physicalSize.dy);
            camera->SetupPerspective(70.f, screenAspect, 0.5f, 2500.f);
            GetScene()->AddCamera(camera);
            GetScene()->SetCurrentCamera(camera);
            entity->AddComponent(new CameraComponent(camera));
        }

        if (isClientOwner || optionsComp->isEnemyPredicted)
        {
            NetworkPredictComponent* npc = new NetworkPredictComponent();
            if (isClientOwner)
            {
                npc->AddPredictedComponent(Type::Instance<ShootCooldownComponent>());
            }
            npc->AddPredictedComponent(Type::Instance<NetworkTransformComponent>());
            entity->AddComponent(npc);
        }

        TransformInterpolationComponent* tic = new TransformInterpolationComponent();
        tic->time = 2.0f;
        entity->AddComponent(tic);
    }

    Entity* tankModel = GetModel(filePath);
    if (IsClient(this))
    {
        NetworkDebugDrawComponent* debugDrawComponent = new NetworkDebugDrawComponent();
        debugDrawComponent->box = tankModel->GetWTMaximumBoundingBoxSlow();
        entity->AddComponent(debugDrawComponent);

        if (optionsComp->options.playerKind.GetId() == PlayerKind::Id::SHOOTER_BOT)
        {
            bool isActor = IsClientOwner(this, entity);
            ShooterBehaviorComponent* behaviorComponent = new ShooterBehaviorComponent(isActor);
            entity->AddComponent(behaviorComponent);
        }
    }

    entity->AddNode(tankModel);
    tankModel->Release();

    if (IsServer(this))
    {
        BoxShapeComponent* boxShape = new BoxShapeComponent();
        const AABBox3 bbox = tankModel->GetWTMaximumBoundingBoxSlow();
        boxShape->SetHalfSize(bbox.GetSize() / 2.0);
        boxShape->SetTypeMask(1);
        boxShape->SetTypeMaskToCollideWith(2);
        boxShape->SetOverrideMass(true);
        boxShape->SetMass(10000.f);
        entity->AddComponent(boxShape);

        DynamicBodyComponent* dynamicBody = new DynamicBodyComponent();
        dynamicBody->SetBodyFlags(PhysicsComponent::eBodyFlags::DISABLE_GRAVITY);
        entity->AddComponent(dynamicBody);
    }
}

void PlayerEntitySystem::FillCarPlayerEntity(DAVA::Entity* entity)
{
    entity->SetName("Car");

    FilePath carModelScenePath("~res:/3d/Jeep/jeepGun-5.sc2");
    ScopedPtr<Scene> carModelScene(new Scene());
    SceneFileV2::eError sceneLoadResult = carModelScene->LoadScene(carModelScenePath);
    DVASSERT(sceneLoadResult == SceneFileV2::ERROR_NO_ERROR);

    // Clone model parts into the entity

    uint32 wheelIds[4]{ 3, 2, 4, 5 };

    for (uint32 i = 0; i < COUNT_OF(wheelIds); ++i)
    {
        Entity* wheel = carModelScene->GetEntityByID(wheelIds[i])->Clone();
        entity->AddNode(wheel);
    }

    Entity* chassis = carModelScene->GetEntityByID(6)->Clone();
    entity->AddNode(chassis);

    Entity* gun = carModelScene->GetEntityByID(7)->Clone();
    entity->AddNode(gun);

    if (IsServer(this))
    {
        NetworkRemoteInputComponent* netRemoteInputComp = new NetworkRemoteInputComponent();
        if (optionsComp->isEnemyPredicted)
        {
            PlayerEntitySystemDetail::CollectRemoteMovingInput(netRemoteInputComp);
        }
        entity->AddComponent(netRemoteInputComp);

        entity->AddComponent(new NetworkInputComponent());
        entity->AddComponent(new NetworkPlayerComponent());

        TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
        NetworkTransformComponent* networkTransform = new NetworkTransformComponent();
        networkTransform->SetPosition(transformComponent->GetPosition());
        networkTransform->SetOrientation(transformComponent->GetRotation());
        entity->AddComponent(networkTransform);

        VehicleCarComponent* carComponent = new VehicleCarComponent();
        entity->AddComponent(carComponent);

        DynamicBodyComponent* carDynamicBody = new DynamicBodyComponent();
        entity->AddComponent(carDynamicBody);

        entity->AddComponent(new NetworkDynamicBodyComponent());
        entity->AddComponent(new NetworkVehicleCarComponent());
    }
    else
    {
        const bool isClientOwner = IsClientOwner(this, entity);
        if (isClientOwner)
        {
            VehicleCarComponent* carComponent = new VehicleCarComponent();
            entity->AddComponent(carComponent);

            DynamicBodyComponent* carDynamicBody = new DynamicBodyComponent();
            entity->AddComponent(carDynamicBody);

            entity->SetName("MyCar");
        }
        else if (!optionsComp->isEnemyPredicted)
        {
            entity->RemoveComponent<VehicleCarComponent>();
            entity->RemoveComponent<DynamicBodyComponent>();

            for (int32 i = 0; i < entity->GetChildrenCount(); ++i)
            {
                Entity* child = entity->GetChild(i);
                child->RemoveComponent<VehicleWheelComponent>();
                child->RemoveComponent<VehicleChassisComponent>();
                child->RemoveComponent<MeshShapeComponent>();
                child->RemoveComponent<BoxShapeComponent>();
            }
        }

        if (isClientOwner || optionsComp->isEnemyPredicted)
        {
            NetworkPredictComponent* npc = new NetworkPredictComponent();
            npc->AddPredictedComponent(Type::Instance<NetworkTransformComponent>());
            entity->AddComponent(npc);
        }

        NetworkDebugDrawComponent* debugDrawComponent = new NetworkDebugDrawComponent();
        debugDrawComponent->box = entity->GetWTMaximumBoundingBoxSlow();
        entity->AddComponent(debugDrawComponent);
    }
}

void PlayerEntitySystem::FillCharacterPlayerEntity(DAVA::Entity* entity)
{
    FilePath filePath("~res:/Sniper_2.sc2");
    entity->SetName("Character");

    if (IsServer(this))
    {
        entity->AddComponent(new NetworkInputComponent());
        entity->AddComponent(new NetworkRemoteInputComponent());
        entity->AddComponent(new NetworkTransformComponent());
        entity->AddComponent(new NetworkPlayerComponent());

        BoxCharacterControllerComponent* controller = new BoxCharacterControllerComponent();
        controller->SetHalfHeight(0.75f);
        controller->SetHalfForwardExtent(3.6f);
        controller->SetHalfSideExtent(1.75f);
        entity->AddComponent(controller);
    }
    else if (IsClientOwner(this, entity))
    {
        entity->SetName("MyCharacter");

        NetworkPredictComponent* npc = new NetworkPredictComponent();
        npc->AddPredictedComponent(Type::Instance<NetworkTransformComponent>());
        entity->AddComponent(npc);

        filePath = "~res:/Sniper_1.sc2";

        BoxCharacterControllerComponent* controller = new BoxCharacterControllerComponent();
        controller->SetHalfHeight(0.75f);
        controller->SetHalfForwardExtent(3.6f);
        controller->SetHalfSideExtent(1.75f);
        entity->AddComponent(controller);
    }

    entity->GetComponent<TransformComponent>()->SetLocalTransform(Vector3(0.0f, 0.0f, 15.0f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f));

    ScopedPtr<Scene> model(new Scene());
    SceneFileV2::eError ret = model->LoadScene(filePath);
    DVASSERT(SceneFileV2::ERROR_NO_ERROR == ret);
    Entity* tankModel = model->GetEntityByID(1)->Clone();
    entity->AddNode(tankModel);
}

DAVA::Entity* PlayerEntitySystem::GetModel(const DAVA::String& pathname) const
{
    if (modelCache.find(pathname) == modelCache.end())
    {
        DAVA::FilePath name(pathname);
        ScopedPtr<Scene> model(new Scene());
        SceneFileV2::eError err = model->LoadScene(DAVA::FilePath(pathname));
        DVASSERT(SceneFileV2::ERROR_NO_ERROR == err);
        modelCache.emplace(pathname, model->GetEntityByID(1)->Clone());
    }

    return modelCache[pathname]->Clone();
}
