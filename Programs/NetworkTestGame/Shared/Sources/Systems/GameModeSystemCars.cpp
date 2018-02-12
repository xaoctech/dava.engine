#include "GameModeSystemCars.h"

#include <Physics/DynamicBodyComponent.h>
#include <Physics/VehicleCarComponent.h>

#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/SwitchComponent.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Render/Highlevel/RenderObject.h>

#include <Input/Mouse.h>
#include <Input/TouchScreen.h>
#include <DeviceManager/DeviceManager.h>

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include "Scene3D/Components/CameraComponent.h"
#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>
#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "Components/SingleComponents/GameModeSingleComponent.h"
#include "Components/PlayerCarComponent.h"
#include "UI/UIControlSystem.h"
#include "Logger/Logger.h"
#include <algorithm>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(GameModeSystemCars)
{
    ReflectionRegistrator<GameModeSystemCars>::Begin()[M::Tags("gm_cars")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &GameModeSystemCars::Process)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::NORMAL, 4.0f)]
    .End();
}

GameModeSystemCars::GameModeSystemCars(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<VehicleCarComponent>())
    , focusedCar(nullptr)
{
    // Load environment
    const char* environmentScenePath = "~res:/3d/Maps/02_desert_train_dt/02_desert_train_dt.sc2";
    ScopedPtr<Scene> environmentScene(new Scene());
    SceneFileV2::eError ret = environmentScene->LoadScene(environmentScenePath);
    DVASSERT(ret == SceneFileV2::eError::ERROR_NO_ERROR);
    NetworkEntitiesSingleComponent* networkEntities = scene->GetSingletonComponent<NetworkEntitiesSingleComponent>();
    Entity* sw = nullptr;
    for (int32 i = 0; i < environmentScene->GetChildrenCount(); ++i)
    {
        Entity* node = environmentScene->GetChild(i);
        scene->AddNode(node);
        SwitchComponent* switchComp = node->GetComponent<SwitchComponent>();
        if (switchComp != nullptr)
        {
            NetworkReplicationComponent* netReplComp = new NetworkReplicationComponent();
            netReplComp->SetNetworkID(NetworkIdSystem::GetEntityIdForStaticObject());
            node->AddComponent(netReplComp);
            networkEntities->RegisterEntity(netReplComp->GetNetworkID(), node);
        }
    }

    // Subscrive to new clients connections
    if (IsServer(this))
    {
        server = scene->GetSingletonComponent<NetworkServerSingleComponent>()->GetServer();
        server->SubscribeOnConnect([this](const Responder& responder) { connectedResponders.push_back(&responder); });
        countdown = 10.f;
    }

    // Setup capturing cursor if we're in windowed mode for camera rotation
    if (GetEngineContext()->deviceManager->GetMouse() != nullptr)
    {
#if !defined(__DAVAENGINE_ANDROID__) && !defined(__DAVAENGINE_IOS__)
        Window* w = GetPrimaryWindow();
        if (w != nullptr)
        {
            w->SetCursorCapture(eCursorCapture::PINNING);
        }
#endif
    }

    // Create camera to use for focused car
    VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
    Size2i physicalWindowSize = vcs->GetPhysicalScreenSize();
    float32 screenAspectRatio = static_cast<float32>(physicalWindowSize.dx) / static_cast<float32>(physicalWindowSize.dy);

    ScopedPtr<Camera> camera(new Camera());
    camera->SetUp(Vector3(0.f, 0.f, 1.f));
    camera->SetPosition(Vector3(0.f, -100.f, 100.f));
    camera->SetTarget(Vector3(0.f, 0.f, 0.f));
    camera->RebuildCameraFromValues();
    camera->SetupPerspective(70.f, screenAspectRatio, 0.5f, 2500.f);

    GetScene()->AddCamera(camera);
    GetScene()->SetCurrentCamera(camera);

    cameraComponent = new CameraComponent(camera);

    Entity* cameraEntity = new Entity();
    cameraEntity->SetName("PlayerCamera");
    cameraEntity->AddComponent(cameraComponent);
    scene->AddNode(cameraEntity);
}

void GameModeSystemCars::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("GameModeSystemCars::Process");
    for (const Responder* responder : connectedResponders)
    {
        OnClientConnected(*responder);
    }
    connectedResponders.clear();

    if (countdown > 0.f)
    {
        countdown -= timeElapsed;
        if (countdown <= 0.f)
        {
            for (int32 i = 0; i < GetScene()->GetChildrenCount(); ++i)
            {
                Entity* node = GetScene()->GetChild(i);
                SwitchComponent* switchComp = node->GetComponent<SwitchComponent>();
                if (switchComp != nullptr)
                {
                    int32 index = (switchComp->GetSwitchIndex() + 1) % 2;
                    switchComp->SetSwitchIndex(index);
                }
            }
            countdown = 10.f;
        }
    }

    // Update camera position
    if (cameraComponent != nullptr && focusedCar != nullptr)
    {
        TransformComponent* focusedCarTransform = focusedCar->GetComponent<TransformComponent>();

        Vector3 cameraOffset = Vector3::Zero;

        Mouse* mouse = GetEngineContext()->deviceManager->GetMouse();
        if (mouse != nullptr && GetPrimaryWindow() != nullptr)
        {
            Matrix4 rotationMatrixZ;
            rotationMatrixZ.BuildRotation(Vector3::UnitZ, mouse->GetPosition().x * PI / 180.0f);

            static Vector3 currentPos = Vector3(0.0f, -15.0f, 7.0f);
            currentPos = currentPos * rotationMatrixZ;
            cameraOffset = currentPos;
        }
        else
        {
            // For touch devices just keep camera behind
            cameraOffset = focusedCarTransform->GetRotation().ApplyToVectorFast(Vector3(10.0f, 0.0f, 3.0f));
        }

        Camera* camera = cameraComponent->GetCamera();
        camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        camera->SetPosition(focusedCarTransform->GetPosition() + cameraOffset);
        camera->SetTarget(focusedCarTransform->GetPosition());
        camera->RebuildCameraFromValues();
    }
}

void GameModeSystemCars::PrepareForRemove()
{
}

void GameModeSystemCars::AddEntity(DAVA::Entity* entity)
{
    cars.insert(entity);
    if (focusedCar == nullptr)
    {
        if ((server != nullptr) ||
            (server == nullptr && IsClientOwner(GetScene(), entity)))
        {
            focusedCar = entity;
        }
    }
}

void GameModeSystemCars::RemoveEntity(DAVA::Entity* entity)
{
    cars.erase(entity);

    if (entity == focusedCar)
    {
        focusedCar = cars.size() > 0 ? *cars.begin() : nullptr;
    }
}

void GameModeSystemCars::OnClientConnected(const Responder& responder)
{
    DVASSERT(server != nullptr);

    NetworkGameModeSingleComponent* networkGameModeComponent = GetScene()->GetSingletonComponent<NetworkGameModeSingleComponent>();
    NetworkPlayerID playerID = networkGameModeComponent->GetNetworkPlayerID(responder.GetToken());
    Entity* playerEntity = networkGameModeComponent->GetPlayerEnity(playerID);
    if (playerEntity == nullptr)
    {
        // Create car as a root object with replication + dynamic body + vehicle components
        // Everything else if filled with PlayerEntitySystem

        Entity* car = new Entity();

        NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent();
        replicationComponent->SetNetworkPlayerID(playerID);
        replicationComponent->SetOwnerTeamID(responder.GetTeamID());
        replicationComponent->SetEntityType(EntityType::VEHICLE);
        car->AddComponent(replicationComponent);

        PlayerCarComponent* carComponent = new PlayerCarComponent();
        car->AddComponent(carComponent);

        // Position the car slightly above the ground
        TransformComponent* carTransform = car->GetComponent<TransformComponent>();
        carTransform->SetLocalTransform(Vector3(0.0f, 0.0f, 14.5f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f));

        GetScene()->AddNode(car);
    }
}
