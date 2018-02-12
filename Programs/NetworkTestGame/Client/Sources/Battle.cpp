#include "Battle.h"
#include "GameClient.h"
#include "Game.h"

#include "Systems/Game01HelloWorld.h"
#include "Systems/GameInputSystem.h"
#include "Systems/GameModeSystem.h"
#include "Systems/GameModeSystemCars.h"
#include "Systems/GameModeSystemCharacters.h"
#include "Systems/GameModeSystemPhysics.h"
#include "Systems/GameShowSystem.h"
#include "Systems/MarkerSystem.h"
#include "Systems/PhysicsProjectileInputSystem.h"
#include "Systems/PhysicsProjectileSystem.h"
#include "Systems/PlayerEntitySystem.h"
#include "Systems/BotSystem.h"
#include "Systems/AI/BotTaskSystem.h"
#include "Systems/AI/ShooterBehaviorSystem.h"

#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include "Components/PhysicsProjectileComponent.h"
#include "Components/ShootComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Components/SingleComponents/GameCollisionSingleComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"

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
#include <NetworkCore/Scene3D/Systems/NetworkPredictSystem2.h>
#include <NetworkCore/Scene3D/Systems/NetworkReplicationSystem2.h>
#include <NetworkCore/Scene3D/Systems/NetworkResimulationSystem.h>
#include <NetworkCore/Scene3D/Systems/SnapshotSystemClient.h>

#include <NetworkCore/Scene3D/Systems/NetworkDebugPredictDrawSystem.h>
#include <NetworkCore/Scene3D/Systems/NetworkDebugDrawSystem.h>

#include <Physics/BoxShapeComponent.h>
#include <Physics/ConvexHullShapeComponent.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/PhysicsSystem.h>
#include <Physics/VehicleCarComponent.h>
#include <Physics/VehicleChassisComponent.h>
#include <Physics/VehicleTankComponent.h>
#include <Physics/VehicleWheelComponent.h>

#include <NetworkPhysics/NetworkPhysicsSystem.h>

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

    const Size2i& physicalSize = GetEngineContext()->uiControlSystem->vcs->GetPhysicalScreenSize();
    float32 screenAspect = static_cast<float32>(physicalSize.dx) / static_cast<float32>(physicalSize.dy);

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
            tags.insert(FastName("shooterbot"));
            break;
        }
        case (PlayerKind::Id::RANDOM_BOT):
        {
            tags.insert(FastName("randombot"));
            break;
        }
        default:
            DVASSERT(0);
        }
    }

    battleScene = new Scene(tags);

    ScopedPtr<Camera> camera(new Camera());
    battleScene->AddCamera(camera);
    battleScene->SetCurrentCamera(camera);

    *battleScene->GetSingletonComponent<BattleOptionsSingleComponent>() = *optionsSingleComp;

    NetworkTimeSingleComponent::SetFrequencyHz(static_cast<float32>(optionsSingleComp->options.freqHz));

    battleScene->GetSingletonComponent<NetworkClientSingleComponent>()->SetClient(gameClient->GetUDPClientPtr());

    battleScene->CreateSystemsByTags();

    if (battleScene->GetSystem<PhysicsSystem>() != nullptr)
    {
        battleScene->GetSystem<PhysicsSystem>()->SetSimulationEnabled(true);
    }

    battleScene->GetSystem<NetworkTimeSystem>()->SetSlowDownFactor(optionsSingleComp->options.slowDownFactor);
}
