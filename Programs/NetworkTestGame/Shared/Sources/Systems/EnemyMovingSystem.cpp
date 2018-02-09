#include "EnemyMovingSystem.h"

#include "Scene3D/Systems/ActionCollectSystem.h"
#include "Systems/GameInputSystem.h"

#include "NetworkCore/Scene3D/Components/NetworkRemoteInputComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"

#include "NetworkCore/Scene3D/Systems/NetworkInputSystem.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

#include <Reflection/ReflectionRegistrator.h>
#include <Debug/ProfilerCPU.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(EnemyMovingSystem)
{
    ReflectionRegistrator<EnemyMovingSystem>::Begin()[M::Tags("network", "input", "client", "enemy_predict")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &EnemyMovingSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 2.0f)]
    .End();
}

EnemyMovingSystem::EnemyMovingSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<NetworkTransformComponent>() | ComponentUtils::MakeMask<NetworkRemoteInputComponent>() | ComponentUtils::MakeMask<NetworkPredictComponent>())
{
    replicationSingleComponent = scene->GetSingletonComponent<NetworkReplicationSingleComponent>();
}

void EnemyMovingSystem::AddEntity(DAVA::Entity* entity)
{
    if (!IsClientOwner(this, entity))
    {
        BaseSimulationSystem::AddEntity(entity);
    }
}

void EnemyMovingSystem::RemoveEntity(DAVA::Entity* entity)
{
    if (!IsClientOwner(this, entity))
    {
        BaseSimulationSystem::RemoveEntity(entity);
    }
}

void EnemyMovingSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("EnemyMovingSystem::ProcessFixed");
    if (!gameInputSystem)
    {
        gameInputSystem = GetScene()->GetSystem<GameInputSystem>();
        DVASSERT(gameInputSystem);
    }

    NetworkTimeSingleComponent* timeComp = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>();
    if (timeComp->IsInitialized())
    {
        for (Entity* entity : entities)
        {
            Simulate(entity);
        }
    }
}

void EnemyMovingSystem::Simulate(Entity* entity)
{
    if (!IsClientOwner(this, entity))
    {
        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), entity);
        if (!allActions.empty() && gameInputSystem)
        {
            const auto& actions = allActions.back();
            gameInputSystem->ApplyDigitalActions(entity, actions.digitalActions, actions.clientFrameId, NetworkTimeSingleComponent::FrameDurationS);
            gameInputSystem->ApplyAnalogActions(entity, actions.analogActions, actions.clientFrameId, NetworkTimeSingleComponent::FrameDurationS);
        }
    }
}
