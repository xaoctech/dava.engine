#include "CreateGameModeSystem.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Logger/Logger.h"

#include "ShooterConstants.h"
#include "ShooterUtils.h"

#include "Systems/Game01HelloWorld.h"
#include "Systems/GameModeSystemPhysics.h"
#include "Systems/GameModeSystemCharacters.h"
#include "Systems/GameModeSystemCars.h"
#include "Systems/GameModeSystem.h"
#include "Systems/PhysicsProjectileSystem.h"
#include "Systems/GameInputSystem.h"
#include "Systems/ShootInputSystem.h"
#include "Systems/ShootSystem.h"
#include "Systems/PhysicsProjectileInputSystem.h"
#include "Systems/MarkerSystem.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"

#include <Physics/PhysicsSystem.h>

#include <NetworkCore/NetworkCoreUtils.h>

#include <Reflection/ReflectionRegistrator.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h>
#include <NetworkCore/Scene3D/Systems/NetworkRemoteInputSystem.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(CreateGameModeSystem)
{
    ReflectionRegistrator<CreateGameModeSystem>::Begin()[M::Tags("network", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &CreateGameModeSystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::NORMAL, 2.0f)]
    .End();
}

CreateGameModeSystem::CreateGameModeSystem(DAVA::Scene* scene)
    : SceneSystem(scene, 0)
{
    DVASSERT(IsClient(scene));
}

void CreateGameModeSystem::Process(DAVA::float32 timeElapsed)
{
    NetworkReplicationSingleComponent::EntityToInfo& entityToInfo = GetScene()->GetSingletonComponent<NetworkReplicationSingleComponent>()->replicationInfo;
    BattleOptionsSingleComponent* optionsComp = GetScene()->GetSingletonComponent<BattleOptionsSingleComponent>();
    if (!isInit && entityToInfo.find(NetworkID::SCENE_ID) != entityToInfo.end())
    {
        CreateGameSystems(optionsComp->gameModeId);
        if (optionsComp->isEnemyPredicted)
        {
            GetScene()->AddTag(FastName("enemy_predict"));
        }

        isInit = true;
    }

    if (GetScene()->HasTag(FastName("input")) && nullptr == remoteInputSystem)
    {
        remoteInputSystem = GetScene()->GetSystem<NetworkRemoteInputSystem>();
        if (remoteInputSystem)
        {
            remoteInputSystem->SetFullInputComparisonFlag(optionsComp->compareInputs);
        }
    }
}

void CreateGameModeSystem::CreateGameSystems(GameMode::Id gameModeId)
{
    UnorderedSet<FastName> tags = { FastName("input") };
    bool isShooterGm = false;
    switch (gameModeId)
    {
    case GameMode::Id::HELLO:
        // broken
        break;
    case GameMode::Id::CHARACTERS:
        tags.insert(FastName("gm_characters"));
        break;
    case GameMode::Id::PHYSICS:
        // broken
        break;
    case GameMode::Id::CARS:
        tags.insert({ FastName("gm_cars") });
        break;
    case GameMode::Id::TANKS:
        tags.insert({ FastName("gm_tanks"), FastName("shoot") });
        break;
    case GameMode::Id::SHOOTER:
        tags.insert(FastName("gm_shooter"));
        isShooterGm = true;
        break;
    default:
        DVASSERT(0);
    }

    if (isShooterGm)
    {
        MarkerSystem* markerSystem = GetScene()->GetSystem<MarkerSystem>();
        markerSystem->SetHealthParams(SHOOTER_CHARACTER_MAX_HEALTH, 0.02f);

        InitializeScene(*GetScene());
    }
    else
    {
        tags.insert({ FastName("gameinput"), FastName("gameshow"), FastName("playerentity") });
    }

    for (auto& tag : tags)
    {
        GetScene()->AddTag(tag);
    }
}
