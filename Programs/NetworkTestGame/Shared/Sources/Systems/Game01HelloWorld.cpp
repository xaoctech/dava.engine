#include "Game01HelloWorld.h"

#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h>
#include <NetworkCore/NetworkCoreUtils.h>

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

#include <Scene3D/Systems/ActionCollectSystem.h>
#include <Scene3D/Scene.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(HelloWorldComponent)
{
    DAVA::ReflectionRegistrator<HelloWorldComponent>::Begin()[DAVA::M::Replicable(DAVA::M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .Field("helloMsg", &HelloWorldComponent::helloMsg)
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(Game01HelloWorld)
{
    ReflectionRegistrator<Game01HelloWorld>::Begin()
    .ConstructorByPointer<Scene*>()
    //.Method("ProcessFixed", &Game01HelloWorld::ProcessFixed)
    .End();
}

Game01HelloWorld::Game01HelloWorld(DAVA::Scene* scene)
    : INetworkInputSimulationSystem(scene, DAVA::ComponentUtils::MakeMask<DAVA::NetworkInputComponent>())
{
    DAVA::uint32 mouseId = DAVA::GetMouseDeviceId();
    //uint32 keyboardId = GetKeyboardDeviceId();

    scene->GetSingletonComponent<ActionsSingleComponent>()->CollectDigitalAction(DAVA::FastName("FIRST_SHOOT"), DAVA::MOUSE_LBUTTON, mouseId);

    if (IsServer(this))
    {
        // server initialization
    }
    else if (IsClient(this))
    {
        // client initialization
    }
    else
    {
        DVASSERT(false);
    }
}

void Game01HelloWorld::ProcessFixed(DAVA::float32 timeElapsed)
{
    for (DAVA::Entity* entity : entities)
    {
        const DAVA::Vector<DAVA::ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), entity);
        for (const auto& actions : allActions)
        {
            ApplyDigitalActions(entity, actions.digitalActions, actions.clientFrameId, timeElapsed);
        }
    }
}

void Game01HelloWorld::ApplyDigitalActions(DAVA::Entity* entity,
                                           const DAVA::Vector<DAVA::FastName>& actions,
                                           DAVA::uint32 clientFrameId,
                                           DAVA::float32 duration) const
{
    for (const DAVA::FastName& actionName : actions)
    {
        if (actionName == DAVA::FastName("FIRST_SHOOT"))
        {
            DAVA::Logger::Info("client fire bullet");
        }
    }
}

void Game01HelloWorld::CreateEntityAfterClientConnected(DAVA::Scene* scene)
{
    DAVA::Logger::Info("create empty entity");
    DAVA::Entity* entity = new DAVA::Entity();
    entity->AddComponent(new DAVA::NetworkReplicationComponent());
    entity->AddComponent(new DAVA::NetworkInputComponent());
    scene->AddNode(entity);
    DAVA::Logger::Info("done empty entity");
}
