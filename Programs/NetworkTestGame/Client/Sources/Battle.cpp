#include "Battle.h"
#include "GameClient.h"
#include "Game.h"

#include "Systems/GameInputSystem.h"
#include "Systems/GameModeSystem.h"
#include "Systems/GameModeSystemCars.h"
#include "Systems/GameShowSystem.h"
#include "Systems/MarkerSystem.h"
#include "Systems/PhysicsProjectileInputSystem.h"
#include "Systems/PhysicsProjectileSystem.h"
#include "Systems/PlayerEntitySystem.h"
#include "Systems/BotSystem.h"
#include "Systems/AI/BotTaskSystem.h"
#include "Systems/AI/ShooterBehaviorSystem.h"
#include "Systems/AI/InvaderBehaviorSystem.h"

#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include "Components/PhysicsProjectileComponent.h"
#include "Components/ShootComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Components/SingleComponents/GameCollisionSingleComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"

#include <Debug/DebugOverlay.h>
#include <Debug/ProfilerCPU.h>
#include <Debug/ProfilerGPU.h>
#include <Debug/ProfilerOverlay.h>
#include <Engine/Engine.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/TransformSystem.h>
#include <UI/Flow/UIFlowContext.h>
#include <UI/UIControlSystem.h>

#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/Scene3D/Systems/NetworkGameModeSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkIdSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkInputSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkTimeSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkTimelineControlSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkRemoteInputSystem.h>

#include <NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemClient.h>
#include <NetworkCore/Scene3D/Systems/NetworkPredictSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkReplicationSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkResimulationSystem.h>
#include <NetworkCore/Scene3D/Systems/SnapshotSystemClient.h>

#include <NetworkCore/Scene3D/Systems/NetworkDebugPredictDrawSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkDebugDrawSystem.h>

#include <Physics/Core/BoxShapeComponent.h>
#include <Physics/Core/ConvexHullShapeComponent.h>
#include <Physics/Core/DynamicBodyComponent.h>
#include <Physics/PhysicsSystem.h>
#include <Physics/Vehicles/VehicleCarComponent.h>
#include <Physics/Vehicles/VehicleChassisComponent.h>
#include <Physics/Vehicles/VehicleTankComponent.h>
#include <Physics/Vehicles/VehicleWheelComponent.h>

#include <NetworkCore/Scene3D/Systems/NetworkTransformFromLocalToNetSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkTransformFromNetToLocalSystem.h>

DAVA_REFLECTION_IMPL(Battle)
{
    DAVA::ReflectionRegistrator<Battle>::Begin()
    .ConstructorByValue()
    .ConstructorByPointer()
    .DestructorByPointer([](Battle* s) { DAVA::SafeDelete(s); })
    .End();
}

void Battle::Initialize(const BattleOptions& options)
{
    using namespace DAVA;

    gameClient = std::make_unique<GameClient>(options.hostName, options.port, options.token);

    optionsSingleComp = new BattleOptionsSingleComponent();
    optionsSingleComp->options = options;
    optionsSingleComp->isSet = true;

    CreateBattleScene();
}

void Battle::Release()
{
    gameClient.reset();
    FreeBattleScene();
}

void Battle::Update(DAVA::float32 frameDelta)
{
    DAVA_PROFILER_CPU_SCOPE("Battle::Update");

    if (gameClient)
    {
        gameClient->Update(frameDelta);
    }

    if (optionsSingleComp->options.playerKind.IsBot())
    {
        battleScene->Update(frameDelta);
    }
}

void Battle::CreateBattleScene()
{
    using namespace DAVA;

    tags.insert({ FastName("client"), FastName("network"), FastName("marker") });

    if (optionsSingleComp->options.isDebug)
    {
        tags.insert(FastName("debugdraw"));
    }

    SetupTestGame();

    gameClient->Setup(GameClient::Options{ battleScene, optionsSingleComp->options.playerKind.IsBot() });
}

void Battle::FreeBattleScene()
{
    battleScene.reset();
}

void Battle::SetupTestGame()
{
    using namespace DAVA;

    if (optionsSingleComp->options.playerKind.IsBot())
    {
        tags.insert(FastName("bot"));

        switch (optionsSingleComp->options.playerKind.GetId())
        {
        case (PlayerKind::Id::SHOOTER_BOT):
        {
            tags.insert({ FastName("shooterbot"), FastName("taskbot") });
            break;
        }
        case (PlayerKind::Id::RANDOM_BOT):
        {
            tags.insert(FastName("randombot"));
            break;
        }
        case (PlayerKind::Id::INVADER_BOT):
        {
            tags.insert({ FastName("invaderbot"), FastName("taskbot") });
            break;
        }
        default:
            DVASSERT(0);
        }
    }

    battleScene = new Scene(tags);
    battleScene->AddComponent(new DAVA::NetworkReplicationComponent(NetworkID::SCENE_ID));

    ScopedPtr<Camera> camera(new Camera());
    battleScene->AddCamera(camera);
    battleScene->SetCurrentCamera(camera);

    *battleScene->GetSingleComponent<BattleOptionsSingleComponent>() = *optionsSingleComp;

    battleScene->GetSingleComponent<NetworkClientSingleComponent>()->SetClient(gameClient->GetUDPClientPtr());
    battleScene->CreateSystemsByTags();

    if (battleScene->GetSystem<PhysicsSystem>() != nullptr)
    {
        battleScene->GetSystem<PhysicsSystem>()->SetSimulationEnabled(true);
    }
}
