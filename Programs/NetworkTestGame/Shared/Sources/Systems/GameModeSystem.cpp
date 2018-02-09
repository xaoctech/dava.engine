#include "GameModeSystem.h"

#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Components/SingleComponents/GameModeSingleComponent.h"
#include "Components/PlayerTankComponent.h"
#include "Systems/PowerupSpawnSystem.h"
#include "Systems/PowerupSystem.h"

#include <Logger/Logger.h>
#include <Scene3D/Components/CameraComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Scene.h>
#include <Systems/GameStunningSystem.h>
#include <Systems/ShootInputSystem.h>
#include <Components/ShootComponent.h>
#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>

#include <Physics/DynamicBodyComponent.h>
#include <Physics/BoxShapeComponent.h>

#include <algorithm>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(GameModeSystem)
{
    ReflectionRegistrator<GameModeSystem>::Begin()[M::Tags("gm_tanks")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &GameModeSystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::NORMAL, 5.0f)]
    .End();
}

GameModeSystem::GameModeSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
    if (IsServer(this))
    {
        server = scene->GetSingletonComponent<NetworkServerSingleComponent>()->GetServer();
        server->SubscribeOnConnect([this](const Responder& responder) {
            connectedResponders.push_back(&responder);
        });
    }
}

void GameModeSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("GameModeSystem::Process");
    for (const Responder* responder : connectedResponders)
    {
        OnClientConnected(*responder);
    }
    connectedResponders.clear();

    NetworkGameModeSingleComponent* netGameModeComponent = GetScene()->GetSingletonComponent<NetworkGameModeSingleComponent>();
    GameModeSingleComponent* gameModeComponent = GetScene()->GetSingletonComponent<GameModeSingleComponent>();

    if (!gameModeComponent->IsMapLoaded())
    {
        // Load map
        const FastName& mapName = netGameModeComponent->GetMapName();
        if (mapName.IsValid())
        {
            gameModeComponent->SetIsMapLoaded(true);
            netGameModeComponent->SetIsLoaded(true);
            Logger::Debug("Map is loaded: %s", mapName.c_str());
        }
    }

    switch (netGameModeComponent->GetGameModeType())
    {
    case GameModeSystem::WAITING:
        ProcessWaitingGameMode(netGameModeComponent);
        break;
    case GameModeSystem::BATTLE:
        ProcessBattleGameMode(netGameModeComponent);
        break;
    default:
        break;
    };
}

void GameModeSystem::OnClientConnected(const Responder& responder)
{
    NetworkGameModeSingleComponent* netGameModeComp = GetScene()->GetSingletonComponent<NetworkGameModeSingleComponent>();
    NetworkPlayerID playerID = netGameModeComp->GetNetworkPlayerID(responder.GetToken());
    Logger::Debug("Token: %s, Player ID: %d", responder.GetToken().c_str(), playerID);
    Entity* playerEntity = netGameModeComp->GetPlayerEnity(playerID);
    if (playerEntity == nullptr)
    {
        Entity* tank = new Entity();
        NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent();
        replicationComponent->SetNetworkPlayerID(playerID);
        replicationComponent->SetOwnerTeamID(responder.GetTeamID());
        tank->AddComponent(replicationComponent);
        tank->AddComponent(new PlayerTankComponent());
        GetScene()->AddNode(tank);
        Logger::Debug("[GameServer] Create Tank %p:%u For Player ID : %d", tank, static_cast<uint32>(replicationComponent->GetNetworkID()), playerID);

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
    }
    else
    {
        NetworkReplicationComponent* replicationComponent = playerEntity->GetComponent<NetworkReplicationComponent>();
        Logger::Debug("[GameServer] Reconnect %p:%u Player ID : %d", playerEntity, static_cast<uint32>(replicationComponent->GetNetworkID()), playerID);
    }
}

void GameModeSystem::ProcessWaitingGameMode(NetworkGameModeSingleComponent* netGameModeComponent)
{
    if (server && netGameModeComponent->GetTokenCount() > 0)
    {
        netGameModeComponent->SetGameModeType(GameModeSystem::BATTLE);
    }
}

void GameModeSystem::ProcessBattleGameMode(NetworkGameModeSingleComponent* netGameModeComponent)
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
