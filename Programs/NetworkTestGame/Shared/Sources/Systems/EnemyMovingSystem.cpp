#include "EnemyMovingSystem.h"

#include "Scene3D/Systems/ActionCollectSystem.h"
#include "Systems/GameInputSystem.h"

#include "NetworkCore/Scene3D/Components/NetworkRemoteInputComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"

#include "NetworkCore/Scene3D/Systems/NetworkInputSystem.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(EnemyMovingSystem)
{
    ReflectionRegistrator<EnemyMovingSystem>::Begin()[M::Tags("network", "gameinput", "client", "enemy_predict")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &EnemyMovingSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 2.0f)]
    .End();
}

EnemyMovingSystem::EnemyMovingSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<NetworkTransformComponent, NetworkRemoteInputComponent, NetworkPredictComponent>())
{
    entityGroup = scene->AquireEntityGroup<NetworkTransformComponent, NetworkRemoteInputComponent, NetworkPredictComponent>();
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
        for (Entity* entity : entityGroup->GetEntities())
        {
            if (IsClientOwner(entity))
            {
                continue;
            }

            const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), entity);
            if (!allActions.empty() && gameInputSystem)
            {
                const auto& actions = allActions.back();
                gameInputSystem->ApplyDigitalActions(entity, actions.digitalActions, actions.clientFrameId, timeElapsed);
                gameInputSystem->ApplyAnalogActions(entity, actions.analogActions, actions.clientFrameId, timeElapsed);
            }
        }
    }
}
