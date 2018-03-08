#include "PlayerEntitySystem.h"

#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/TransformInterpolationComponent.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/SkeletonComponent.h>

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
#include "Visibility/ObserverComponent.h"
#include "Visibility/ObservableComponent.h"
#include "Visibility/SimpleVisibilityShapeComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"

#include <Physics/PhysicsSystem.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/VehicleCarComponent.h>
#include <Physics/VehicleWheelComponent.h>
#include <Physics/VehicleChassisComponent.h>
#include <Physics/BoxShapeComponent.h>
#include <Physics/MeshShapeComponent.h>
#include <Physics/ConvexHullShapeComponent.h>
#include <Physics/BoxCharacterControllerComponent.h>
#include <Physics/CapsuleCharacterControllerComponent.h>
#include <Physics/PhysicsUtils.h>

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
    .Method("Process", &PlayerEntitySystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 24.0f)]
    .End();
}

PlayerEntitySystem::PlayerEntitySystem(DAVA::Scene* scene)
    : SceneSystem(scene, ComponentMask())
    , tanksSubscriber(scene->AquireEntityGroupOnAdd(scene->AquireEntityGroup<PlayerTankComponent>(), this))
    , carsSubscriber(scene->AquireEntityGroupOnAdd(scene->AquireEntityGroup<PlayerCarComponent>(), this))
{
    optionsComp = scene->GetSingleComponent<BattleOptionsSingleComponent>();
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
}

void PlayerEntitySystem::FillTankPlayerEntity(DAVA::Entity* entity)
{
    PlayerTankComponent* ptc = entity->GetComponent<PlayerTankComponent>();
    NetworkPlayerID playerId = ptc->playerId;

    Logger::Debug("[PlayerEntitySystem::Process] %d tank factory, for player %d", entity->GetID(), playerId);

    String filePath("~res:/Sniper_2.sc2");
    entity->SetName(Format("Tank %d", playerId).c_str());

    if (IsServer(this))
    {
        NetworkID tankId = NetworkID::CreatePlayerOwnId(playerId);
        NetworkReplicationComponent* nrc = new NetworkReplicationComponent(tankId);

        nrc->SetForReplication<NetworkPlayerComponent>(M::Privacy::PRIVATE);
        nrc->SetForReplication<NetworkInputComponent>(M::Privacy::PRIVATE);
        nrc->SetForReplication<NetworkRemoteInputComponent>(M::Privacy::PUBLIC);
        nrc->SetForReplication<NetworkTransformComponent>(M::Privacy::PUBLIC);
        nrc->SetForReplication<PlayerTankComponent>(M::Privacy::PUBLIC);
        nrc->SetForReplication<HealthComponent>(M::Privacy::PUBLIC);
        nrc->SetForReplication<ShootCooldownComponent>(M::Privacy::PRIVATE);
        nrc->SetForReplication<GameStunnableComponent>(M::Privacy::PUBLIC);

        NetworkRemoteInputComponent* netRemoteInputComp = new NetworkRemoteInputComponent();
        if (optionsComp->isEnemyPredicted)
        {
            PlayerEntitySystemDetail::CollectRemoteMovingInput(netRemoteInputComp);
        }

        entity->AddComponent(nrc);
        entity->AddComponent(netRemoteInputComp);
        entity->AddComponent(new NetworkInputComponent());
        entity->AddComponent(new ShootCooldownComponent());
        entity->AddComponent(new NetworkTransformComponent());
        entity->AddComponent(new NetworkPlayerComponent());
        entity->AddComponent(new GameStunnableComponent());
        entity->AddComponent(new HealthComponent());
        entity->AddComponent(new PowerupCatcherComponent());
        entity->AddComponent(new NetworkTrafficLimitComponent());
        entity->AddComponent(new ObserverComponent());
        entity->AddComponent(new ObservableComponent());
        entity->AddComponent(new SimpleVisibilityShapeComponent());
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
            ComponentMask predictionMask;
            predictionMask.Set<NetworkTransformComponent>();

            if (isClientOwner)
            {
                predictionMask.Set<ShootCooldownComponent>();
            }

            NetworkPredictComponent* npc = new NetworkPredictComponent(predictionMask);
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

    Entity* carReference = carModelScene->GetEntityByID(2);
    uint32 numWheels = carReference->GetComponentCount<VehicleWheelComponent>();

    entity->AddComponent(carReference->GetComponent<RenderComponent>()->Clone(entity));
    entity->AddComponent(carReference->GetComponent<SkeletonComponent>()->Clone(entity));

    if (IsServer(this))
    {
        PlayerCarComponent* pcc = entity->GetComponent<PlayerCarComponent>();
        NetworkPlayerID playerID = pcc->playerId;

        NetworkID carId = NetworkID::CreatePlayerOwnId(playerID);
        NetworkReplicationComponent* nrc = new NetworkReplicationComponent(carId);

        nrc->SetForReplication<NetworkPlayerComponent>(M::Privacy::PRIVATE);
        nrc->SetForReplication<NetworkInputComponent>(M::Privacy::PRIVATE);
        nrc->SetForReplication<NetworkRemoteInputComponent>(M::Privacy::PUBLIC);
        nrc->SetForReplication<NetworkTransformComponent>(M::Privacy::PUBLIC);
        nrc->SetForReplication<PlayerCarComponent>(M::Privacy::PUBLIC);
        nrc->SetForReplication<DynamicBodyComponent>(M::Privacy::PRIVATE);
        nrc->SetForReplication<BoxShapeComponent>(M::Privacy::PRIVATE);
        nrc->SetForReplication<ConvexHullShapeComponent>(M::Privacy::PRIVATE);
        nrc->SetForReplication<VehicleCarComponent>(M::Privacy::PRIVATE);
        nrc->SetForReplication<VehicleWheelComponent>(M::Privacy::PRIVATE);
        nrc->SetForReplication<VehicleChassisComponent>(M::Privacy::PRIVATE);

        NetworkRemoteInputComponent* netRemoteInputComp = new NetworkRemoteInputComponent();
        if (optionsComp->isEnemyPredicted)
        {
            PlayerEntitySystemDetail::CollectRemoteMovingInput(netRemoteInputComp);
        }

        entity->AddComponent(carReference->GetComponent<DynamicBodyComponent>()->Clone(entity));
        entity->AddComponent(carReference->GetComponent<VehicleCarComponent>()->Clone(entity));
        entity->AddComponent(carReference->GetComponent<VehicleChassisComponent>()->Clone(entity));
        entity->AddComponent(carReference->GetComponent<BoxShapeComponent>()->Clone(entity));
        for (uint32 i = 0; i < numWheels; ++i)
        {
            entity->AddComponent(carReference->GetComponent<VehicleWheelComponent>(i)->Clone(entity));
            entity->AddComponent(carReference->GetComponent<ConvexHullShapeComponent>(i)->Clone(entity));
        }

        entity->AddComponent(nrc);
        entity->AddComponent(netRemoteInputComp);
        entity->AddComponent(new NetworkInputComponent());
        entity->AddComponent(new NetworkPlayerComponent());

        TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
        NetworkTransformComponent* networkTransform = new NetworkTransformComponent();
        networkTransform->SetPosition(transformComponent->GetPosition());
        networkTransform->SetOrientation(transformComponent->GetRotation());
        entity->AddComponent(networkTransform);

        entity->AddComponent(new ObserverComponent());
        entity->AddComponent(new ObservableComponent());
        entity->AddComponent(new SimpleVisibilityShapeComponent());
    }
    else
    {
        const bool isClientOwner = IsClientOwner(this, entity);
        if (isClientOwner)
        {
            entity->SetName("MyCar");
        }

        if (isClientOwner || optionsComp->isEnemyPredicted)
        {
            ComponentMask predictionMask;
            predictionMask.Set<NetworkTransformComponent>();
            predictionMask.Set<DynamicBodyComponent>();
            predictionMask.Set<BoxShapeComponent>();
            predictionMask.Set<ConvexHullShapeComponent>();
            predictionMask.Set<VehicleWheelComponent>();
            predictionMask.Set<VehicleChassisComponent>();
            predictionMask.Set<VehicleCarComponent>();

            NetworkPredictComponent* npc = new NetworkPredictComponent(predictionMask);
            entity->AddComponent(npc);
        }

        NetworkDebugDrawComponent* debugDrawComponent = new NetworkDebugDrawComponent();
        debugDrawComponent->box = entity->GetWTMaximumBoundingBoxSlow();
        entity->AddComponent(debugDrawComponent);
    }
}

DAVA::Entity* PlayerEntitySystem::GetModel(const DAVA::String& pathname) const
{
    if (modelCache.find(pathname) == modelCache.end())
    {
        DAVA::FilePath name(pathname);
        ScopedPtr<Scene> model(new Scene(0));
        SceneFileV2::eError err = model->LoadScene(DAVA::FilePath(pathname));
        DVASSERT(SceneFileV2::ERROR_NO_ERROR == err);
        modelCache.emplace(pathname, model->GetEntityByID(1)->Clone());
    }

    return modelCache[pathname]->Clone();
}