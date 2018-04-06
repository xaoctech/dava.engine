#include "GameModeSystem.h"

#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Components/SingleComponents/GameModeSingleComponent.h"
#include "Components/PlayerTankComponent.h"
#include "Systems/PowerupSpawnSystem.h"
#include "Systems/PowerupSystem.h"
#include "Components/AI/ShooterBehaviorComponent.h"

#include <Logger/Logger.h>
#include <Scene3D/Components/CameraComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Scene.h>
#include <Systems/GameStunningSystem.h>
#include <Systems/ShootInputSystem.h>
#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkFactoryComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>

#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerConnectionsSingleComponent.h>
#include <NetworkCore/NetworkFactoryUtils.h>

#include <Physics/Core/DynamicBodyComponent.h>
#include <Physics/Core/BoxShapeComponent.h>

#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <UI/UIControlSystem.h>
#include <algorithm>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(GameModeSystem)
{
    ReflectionRegistrator<GameModeSystem>::Begin()[M::Tags("gm_tanks")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &GameModeSystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::NORMAL, 5.0f)]
    .Method("ProcessFixed", &GameModeSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 0.1f)]
    .End();
}

GameModeSystem::GameModeSystem(Scene* scene)
    : SceneSystem(scene, ComponentMask())
{
    netConnectionsComp = scene->GetSingleComponent<NetworkServerConnectionsSingleComponent>();
    netGameModeComp = scene->GetSingleComponent<NetworkGameModeSingleComponent>();
    gameModeComp = scene->GetSingleComponent<GameModeSingleComponent>();
}

void GameModeSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("GameModeSystem::Process");

    if (!gameModeComp->IsMapLoaded())
    {
        // Load map
        const FastName& mapName = netGameModeComp->GetMapName();
        if (mapName.IsValid())
        {
            gameModeComp->SetIsMapLoaded(true);
            netGameModeComp->SetIsLoaded(true);
            Logger::Debug("Map is loaded: %s", mapName.c_str());
        }
    }

    switch (netGameModeComp->GetGameModeType())
    {
    case GameModeSystem::WAITING:
        ProcessWaitingGameMode();
        break;
    case GameModeSystem::BATTLE:
        ProcessBattleGameMode();
        break;
    default:
        break;
    };
}

void GameModeSystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    for (const FastName& justConnectedToken : netConnectionsComp->GetJustConnectedTokens())
    {
        OnClientConnected(justConnectedToken);
    }
}

void GameModeSystem::OnClientConnected(const FastName& token)
{
    NetworkGameModeSingleComponent* netGameModeComp = GetScene()->GetSingleComponent<NetworkGameModeSingleComponent>();
    NetworkPlayerID playerId = netGameModeComp->GetNetworkPlayerID(token);
    Logger::Debug("Token: %s, Player ID: %d", token.c_str(), playerId);
    Entity* playerEntity = netGameModeComp->GetPlayerEnity(playerId);
    if (playerEntity == nullptr)
    {
        static bool hasCamera = false;
        if (!hasCamera)
        {
            hasCamera = true;
            Camera* camera = GetScene()->GetCurrentCamera();
            camera->SetUp(Vector3(0.f, 0.f, 1.f));
            camera->SetPosition(Vector3(0.f, -100.f, 100.f));
            camera->SetTarget(Vector3(0.f, 0.f, 0.f));
            camera->RebuildCameraFromValues();
        }

        NetworkFactoryComponent* fc = CreateEntityWithFactoryComponent("Vehicles/Tank", NetworkID::CreatePlayerOwnId(playerId));
        fc->SetupAfterInit([this, playerId](Entity* tank) {
            BoxShapeComponent* boxShape = tank->GetComponent<BoxShapeComponent>();
            if (boxShape)
            {
                Entity* rocketModel = tank->FindByName("Model");
                const AABBox3 bbox = rocketModel->GetWTMaximumBoundingBoxSlow();
                boxShape->SetHalfSize(bbox.GetSize() / 2.0);
            }

            DynamicBodyComponent* dynamicBody = tank->GetComponent<DynamicBodyComponent>();
            if (dynamicBody)
            {
                dynamicBody->SetBodyFlags(PhysicsComponent::eBodyFlags::DISABLE_GRAVITY);
            }

            Logger::Debug("[GameServer] Create Tank %p for Player ID : %d", tank, playerId);
        });

        GetScene()->AddNode(fc->GetEntity());
    }
    else
    {
        NetworkReplicationComponent* replicationComponent = playerEntity->GetComponent<NetworkReplicationComponent>();
        Logger::Debug("[GameServer] Reconnect %p:%u Player ID : %d", playerEntity, static_cast<uint32>(replicationComponent->GetNetworkID()), playerId);
    }
}

void GameModeSystem::ProcessWaitingGameMode()
{
    if (IsServer(this) && netGameModeComp->GetTokenCount() > 0)
    {
        netGameModeComp->SetGameModeType(GameModeSystem::BATTLE);
    }
}

void GameModeSystem::ProcessBattleGameMode()
{
    //if (!GetScene()->GetSystem<PowerupSpawnSystem>())
    //{
    //    PowerupSpawnSystem* powerupSpawnSystem = new PowerupSpawnSystem(GetScene());
    //    GetScene()->AddSystem(powerupSpawnSystem, 0);
    //}
    //if (IsServer(GetScene()))
    //{
    //    if (!GetScene()->GetSystem<PowerupSystem>())
    //    {
    //        PowerupSystem* powerupSystem = new PowerupSystem(GetScene());
    //        GetScene()->AddSystem(powerupSystem, 0);
    //    }
    //}
}
