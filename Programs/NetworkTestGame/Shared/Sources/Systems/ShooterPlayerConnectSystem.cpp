#include "Systems/ShooterPlayerConnectSystem.h"
#include "Components/ShooterRoleComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterPlayerConnectSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterPlayerConnectSystem>::Begin()[M::Tags("gm_shooter", "server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterPlayerConnectSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 9.0f)]
    .End();
}

ShooterPlayerConnectSystem::ShooterPlayerConnectSystem(DAVA::Scene* scene)
    : SceneSystem(scene, 0)
{
    DAVA::IServer* server = scene->GetSingletonComponent<DAVA::NetworkServerSingleComponent>()->GetServer();

    DVASSERT(server != nullptr);

    server->SubscribeOnConnect(DAVA::MakeFunction(this, &ShooterPlayerConnectSystem::OnPlayerConnected));
}

void ShooterPlayerConnectSystem::OnPlayerConnected(const DAVA::Responder& responder)
{
    newPlayers.push_back(&responder);
}

void ShooterPlayerConnectSystem::ProcessFixed(DAVA::float32 dt)
{
    for (const DAVA::Responder* player : newPlayers)
    {
        DVASSERT(player != nullptr);
        AddPlayerToScene(*player);
    }

    newPlayers.clear();
}

void ShooterPlayerConnectSystem::PrepareForRemove()
{
}

void ShooterPlayerConnectSystem::AddPlayerToScene(const DAVA::Responder& responder)
{
    using namespace DAVA;

    NetworkGameModeSingleComponent* networkGameModeSingleComponent = GetScene()->GetSingletonComponent<NetworkGameModeSingleComponent>();
    NetworkPlayerID playerId = networkGameModeSingleComponent->GetNetworkPlayerID(responder.GetToken());
    Entity* playerEntity = networkGameModeSingleComponent->GetPlayerEnity(playerId);
    if (playerEntity == nullptr)
    {
        Entity* player = new Entity();

        // This system only creates replication & role components, ShooterEntityFillSystem is responsible for the rest

        NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent();
        replicationComponent->SetNetworkPlayerID(playerId);
        replicationComponent->SetOwnerTeamID(responder.GetTeamID());
        replicationComponent->SetEntityType(EntityType::VEHICLE); // Workaround for GameVisbilitySystem to work properly
        player->AddComponent(replicationComponent);

        player->AddComponent(new NetworkPlayerComponent());

        ShooterRoleComponent* roleComponent = new ShooterRoleComponent();
        roleComponent->SetRole(ShooterRoleComponent::Role::Player);
        player->AddComponent(roleComponent);

        GetScene()->AddNode(player);
    }
}
