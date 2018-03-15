#include "InvaderConnectSystem.h"

#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Components/SingleComponents/GameModeSingleComponent.h"
#include "Components/PlayerInvaderComponent.h"
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
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkConnectionsSingleComponent.h>

DAVA_VIRTUAL_REFLECTION_IMPL(InvaderConnectSystem)
{
    ReflectionRegistrator<InvaderConnectSystem>::Begin()[M::Tags("gm_invaders", "server")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &InvaderConnectSystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::NORMAL, 7.0f)]
    .End();
}

InvaderConnectSystem::InvaderConnectSystem(Scene* scene)
    : SceneSystem(scene, ComponentMask())
{
    if (IsServer(this))
    {
        netConnectionsComp = scene->GetSingleComponent<NetworkConnectionsSingleComponent>();
        DVASSERT(netConnectionsComp);
    }
}

void InvaderConnectSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("GameModeSystem::Process");
    if (IsServer(this))
    {
        for (const FastName& justConnectedToken : netConnectionsComp->GetJustConnectedTokens())
        {
            OnClientConnected(justConnectedToken);
        }
    }

    NetworkGameModeSingleComponent* netGameModeComponent = GetScene()->GetSingleComponent<NetworkGameModeSingleComponent>();
    if (nullptr != netGameModeComponent)
    {
        GameModeSingleComponent* gameModeComponent = GetScene()->GetSingleComponent<GameModeSingleComponent>();
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
    }
}

void InvaderConnectSystem::OnClientConnected(const FastName& token)
{
    NetworkGameModeSingleComponent* netGameModeComp = GetScene()->GetSingleComponent<NetworkGameModeSingleComponent>();
    NetworkPlayerID playerID = netGameModeComp->GetNetworkPlayerID(token);
    Logger::Debug("Token: %s, Player ID: %d", token.c_str(), playerID);
    Entity* playerEntity = netGameModeComp->GetPlayerEnity(playerID);
    if (playerEntity == nullptr)
    {
        Entity* invader = new Entity();
        PlayerInvaderComponent* invComp = new PlayerInvaderComponent();
        invComp->playerId = playerID;

        invader->AddComponent(invComp);
        GetScene()->AddNode(invader);

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
        Logger::Debug("[GameServer] Reconnect Invader %p:%d Player ID : %d",
                      playerEntity, static_cast<uint32>(replicationComponent->GetNetworkID()), playerID);
    }
}
