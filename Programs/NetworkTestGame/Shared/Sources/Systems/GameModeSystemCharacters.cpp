#include "GameModeSystemCharacters.h"

#include "Components/PlayerCharacterComponent.h"
#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Components/SingleComponents/GameModeSingleComponent.h"
#include "Systems/PhysicsProjectileInputSystem.h"

#include <Logger/Logger.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Material/NMaterial.h>
#include <Render/Material/NMaterialNames.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Scene3D/Components/CameraComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Scene.h>
#include <Systems/GameStunningSystem.h>
#include <Systems/ShootInputSystem.h>
#include <UI/UIControlSystem.h>
#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Input/Mouse.h>
#include <DeviceManager/DeviceManager.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>

#include <Physics/BoxCharacterControllerComponent.h>
#include <Physics/PhysicsSystem.h>

#include <algorithm>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(GameModeSystemCharacters)
{
    ReflectionRegistrator<GameModeSystemCharacters>::Begin()[M::Tags("gm_characters")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &GameModeSystemCharacters::Process)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::NORMAL, 3.0f)]
    .End();
}

GameModeSystemCharacters::GameModeSystemCharacters(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<BoxCharacterControllerComponent>()) // Handle box character controllers only for now
    , focusedCharacter(nullptr)
{
    ScopedPtr<Scene> sceneToAdd(new Scene());
    SceneFileV2::eError ret = sceneToAdd->LoadScene("~res:/3d/Maps/02_desert_train_dt/02_desert_train_dt.sc2");
    DVASSERT(ret == SceneFileV2::eError::ERROR_NO_ERROR);
    for (int32 i = 0; i < sceneToAdd->GetChildrenCount(); ++i)
    {
        scene->AddNode(sceneToAdd->GetChild(i));
    }

    if (IsServer(this))
    {
        server = scene->GetSingletonComponent<NetworkServerSingleComponent>()->GetServer();
        server->SubscribeOnConnect([this](const Responder& responder) { connectedResponders.push_back(&responder); });
    }

    // Setup capturing cursor if we're in windowed mode for camera rotation
    if (GetEngineContext()->deviceManager->GetMouse() != nullptr)
    {
        Window* w = GetPrimaryWindow();
        if (w != nullptr)
        {
            w->SetCursorCapture(eCursorCapture::PINNING);
        }
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
}

void GameModeSystemCharacters::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("GameModeSystem::Process");
    for (const Responder* responder : connectedResponders)
    {
        OnClientConnected(*responder);
    }
    connectedResponders.clear();

    // Update camera position
    if (cameraComponent != nullptr && focusedCharacter != nullptr)
    {
        TransformComponent* focusedCarTransform = focusedCharacter->GetComponent<TransformComponent>();

        Vector3 cameraOffset = Vector3::Zero;

        Mouse* mouse = GetEngineContext()->deviceManager->GetMouse();
        if (mouse != nullptr && GetPrimaryWindow() != nullptr)
        {
            Matrix4 rotationMatrixZ;
            rotationMatrixZ.BuildRotation(Vector3::UnitZ, mouse->GetPosition().x * PI / 180.0f);

            static Vector3 currentPos = Vector3(0.0f, -30.0f, 15.0f);
            currentPos = currentPos * rotationMatrixZ;
            cameraOffset = currentPos;
        }
        else
        {
            // For touch devices just keep camera behind
            cameraOffset = focusedCarTransform->GetRotation().ApplyToVectorFast(Vector3(-30.0f, 0.0f, 15.0f));
        }

        Camera* camera = cameraComponent->GetCamera();
        camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        camera->SetPosition(focusedCarTransform->GetPosition() + cameraOffset);
        camera->SetTarget(focusedCarTransform->GetPosition());
        camera->RebuildCameraFromValues();
    }
}

void GameModeSystemCharacters::PrepareForRemove()
{
}
void GameModeSystemCharacters::AddEntity(DAVA::Entity* entity)
{
    characters.insert(entity);
    if (focusedCharacter == nullptr)
    {
        if ((server != nullptr) ||
            (server == nullptr && /*IsClientOwner(GetScene(), entity)*/ true))
        {
            focusedCharacter = entity;
        }
    }
}

void GameModeSystemCharacters::RemoveEntity(DAVA::Entity* entity)
{
    characters.erase(entity);

    if (entity == focusedCharacter)
    {
        focusedCharacter = characters.size() > 0 ? *characters.begin() : nullptr;
    }
}

void GameModeSystemCharacters::OnClientConnected(const Responder& responder)
{
    DVASSERT(server != nullptr);

    NetworkGameModeSingleComponent* netGameModeComp = GetScene()->GetSingletonComponent<NetworkGameModeSingleComponent>();
    NetworkPlayerID playerID = netGameModeComp->GetNetworkPlayerID(responder.GetToken());
    Entity* playerEntity = netGameModeComp->GetPlayerEnity(playerID);
    if (playerEntity == nullptr)
    {
        Entity* characterEntity = new Entity();

        NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent();
        replicationComponent->SetNetworkPlayerID(playerID);
        replicationComponent->SetOwnerTeamID(responder.GetTeamID());
        replicationComponent->SetEntityType(EntityType::VEHICLE);
        characterEntity->AddComponent(replicationComponent);

        PlayerCharacterComponent* characterComponent = new PlayerCharacterComponent();
        characterEntity->AddComponent(characterComponent);

        GetScene()->AddNode(characterEntity);
    }
}
