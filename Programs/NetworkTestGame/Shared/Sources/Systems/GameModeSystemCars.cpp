#include "GameModeSystemCars.h"

#include <Physics/Core/DynamicBodyComponent.h>
#include <Physics/Vehicles/VehicleCarComponent.h>

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
#include "Visibility/ObservableComponent.h"
#include "Visibility/SimpleVisibilityShapeComponent.h"
#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerConnectionsSingleComponent.h>
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
    .Method("Process", &GameModeSystemCars::Process)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::NORMAL, 4.0f)]
    .Method("ProcessFixed", &GameModeSystemCars::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 0.2f)]
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
    NetworkEntitiesSingleComponent* networkEntities = scene->GetSingleComponent<NetworkEntitiesSingleComponent>();
    Entity* sw = nullptr;
    for (int32 i = 0; i < environmentScene->GetChildrenCount(); ++i)
    {
        Entity* node = environmentScene->GetChild(i);
        SwitchComponent* switchComp = node->GetComponent<SwitchComponent>();
        if (switchComp != nullptr)
        {
            NetworkID switchId = NetworkID::CreateStaticId(node->GetID());

            NetworkReplicationComponent* netReplComp = new NetworkReplicationComponent(switchId);
            netReplComp->SetForReplication(switchComp->GetType(), M::Privacy::PUBLIC);
            node->AddComponent(netReplComp);
            node->AddComponent(new SimpleVisibilityShapeComponent());
            node->AddComponent(new ObservableComponent());
            networkEntities->RegisterEntity(netReplComp->GetNetworkID(), node);
        }
        scene->AddNode(node);
    }

    // Subscrive to new clients connections
    if (IsServer(this))
    {
        netConnectionsComp = scene->GetSingleComponent<NetworkServerConnectionsSingleComponent>();
        DVASSERT(netConnectionsComp);
        countdown = 2.f;
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
            countdown = 2.f;
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

void GameModeSystemCars::ProcessFixed(DAVA::float32 timeElapsed)
{
    if (IsServer(this))
    {
        for (const FastName& justConnectedToken : netConnectionsComp->GetJustConnectedTokens())
        {
            OnClientConnected(justConnectedToken);
        }
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
        if (IsServer(this) || IsClientOwner(GetScene(), entity))
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

void GameModeSystemCars::OnClientConnected(const FastName& token)
{
    NetworkGameModeSingleComponent* networkGameModeComponent = GetScene()->GetSingleComponent<NetworkGameModeSingleComponent>();
    NetworkPlayerID playerID = networkGameModeComponent->GetNetworkPlayerID(token);
    Entity* playerEntity = networkGameModeComponent->GetPlayerEnity(playerID);
    if (playerEntity == nullptr)
    {
        // Create car as a root object with replication + dynamic body + vehicle components
        // Everything else if filled with PlayerEntitySystem

        Entity* car = new Entity();

        PlayerCarComponent* carComponent = new PlayerCarComponent();
        carComponent->playerId = playerID;
        car->AddComponent(carComponent);

        // Position the car slightly above the ground
        TransformComponent* carTransform = car->GetComponent<TransformComponent>();
        carTransform->SetLocalTransform(Vector3(0.0f, 0.0f, 14.5f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f));

        GetScene()->AddNode(car);
    }
}
