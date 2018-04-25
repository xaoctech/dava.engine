#include "Battle.h"
#include "GameClient.h"
#include "Game.h"

#include "Components/SingleComponents/GameCollisionSingleComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"

#include <Debug/ProfilerCPU.h>

#include <Reflection/Reflection.h>
#include <Reflection/ReflectedObject.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h>

#include <Physics/PhysicsSystem.h>

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

    tags.insert({ FastName("base"), FastName("physics"), FastName("client"), FastName("network"), FastName("marker") });

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
        case (PlayerKind::Id::TANK_BATTLE_ROYALE_BOT):
        {
            tags.insert({ FastName("tankbot"), FastName("taskbot") });
            break;
        }
        case (PlayerKind::Id::SHOOTER_BATTLE_ROYALE_BOT):
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

    const bool createSystems = false;
    battleScene = new Scene(tags, createSystems);
    battleScene->AddComponent(new DAVA::NetworkReplicationComponent(NetworkID::SCENE_ID));

    ScopedPtr<Camera> camera(new Camera());
    battleScene->AddCamera(camera);
    battleScene->SetCurrentCamera(camera);

    *battleScene->GetSingleComponent<BattleOptionsSingleComponent>() = *optionsSingleComp;

    battleScene->GetSingleComponent<NetworkClientSingleComponent>()->SetClient(gameClient->GetUDPClientPtr());
    battleScene->CreateDelayedSystems();

    if (battleScene->GetSystem<PhysicsSystem>() != nullptr)
    {
        battleScene->GetSystem<PhysicsSystem>()->SetSimulationEnabled(true);
    }
}
