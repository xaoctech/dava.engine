#include "Systems/ShooterPlayerConnectSystem.h"
#include "Components/ShooterRoleComponent.h"
#include "Visibility/ObserverComponent.h"
#include "Visibility/ObservableComponent.h"
#include "Visibility/CharacterVisibilityShapeComponent.h"
#include "ShooterConstants.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerConnectionsSingleComponent.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterPlayerConnectSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterPlayerConnectSystem>::Begin()[M::Tags("gm_shooter", "server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterPlayerConnectSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 9.0f)]
    .End();
}

ShooterPlayerConnectSystem::ShooterPlayerConnectSystem(DAVA::Scene* scene)
    : SceneSystem(scene, DAVA::ComponentMask())
{
    netConnectionsComp = scene->GetSingleComponent<DAVA::NetworkServerConnectionsSingleComponent>();
    DVASSERT(netConnectionsComp);
}

void ShooterPlayerConnectSystem::ProcessFixed(DAVA::float32 dt)
{
    for (const DAVA::FastName& justConnectedToken : netConnectionsComp->GetJustConnectedTokens())
    {
        AddPlayerToScene(justConnectedToken);
    }
}

void ShooterPlayerConnectSystem::PrepareForRemove()
{
}

void ShooterPlayerConnectSystem::AddPlayerToScene(const DAVA::FastName& token)
{
    // This system only creates replication & role components, ShooterEntityFillSystem is responsible for the rest

    using namespace DAVA;

    NetworkGameModeSingleComponent* networkGameModeSingleComponent = GetScene()->GetSingleComponent<NetworkGameModeSingleComponent>();
    NetworkPlayerID playerId = networkGameModeSingleComponent->GetNetworkPlayerID(token);
    Entity* playerEntity = networkGameModeSingleComponent->GetPlayerEnity(playerId);
    if (playerEntity == nullptr)
    {
        Entity* player = new Entity();

        ShooterRoleComponent* roleComponent = new ShooterRoleComponent();
        roleComponent->playerID = playerId;
        roleComponent->SetRole(ShooterRoleComponent::Role::Player);
        player->AddComponent(roleComponent);

        ObserverComponent* observer = player->GetOrCreateComponent<ObserverComponent>();
        observer->maxVisibilityRadius = SHOOTER_MAX_VISIBILITY_RADIUS;
        observer->unconditionalVisibilityRadius = SHOOTER_UNCONDITIONAL_VISIBILITY_RADIUS;
        player->AddComponent(new ObservableComponent());

        CharacterVisibilityShapeComponent* charVisShape = new CharacterVisibilityShapeComponent();
        charVisShape->height = SHOOTER_CHARACTER_VISIBILITY_HEIGHT;
        player->AddComponent(charVisShape);

        GetScene()->AddNode(player);
    }
}
