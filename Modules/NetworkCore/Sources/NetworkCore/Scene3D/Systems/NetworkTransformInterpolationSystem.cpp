#include "NetworkTransformInterpolationSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"

#include "Scene3D/Components/TransformInterpolationComponent.h"
#include "Scene3D/Components/TransformComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkTransformInterpolationSystem)
{
    ReflectionRegistrator<NetworkTransformInterpolationSystem>::Begin()[M::Tags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkTransformInterpolationSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 111.0f)]
    .End();
}

NetworkTransformInterpolationSystem::NetworkTransformInterpolationSystem(Scene* scene)
    : BaseSimulationSystem(scene, 0)
    , fixedInterpolationGroup(scene->AquireComponentGroup<TransformInterpolationComponent, TransformInterpolationComponent, NetworkPredictComponent>())
    , fixedInterpolationPendingAdd(fixedInterpolationGroup)
{
}

void NetworkTransformInterpolationSystem::ReSimulationStart(Entity* entity, uint32 frameId)
{
    for (TransformInterpolationComponent* tic : fixedInterpolationGroup->components)
    {
        tic->state = InterpolationState::DISABLED;
    }
}

void NetworkTransformInterpolationSystem::ReSimulationEnd(Entity* entity)
{
    for (TransformInterpolationComponent* tic : fixedInterpolationGroup->components)
    {
        TransformComponent* tc = tic->GetEntity()->GetComponent<TransformComponent>();
        tic->SetNewTransform(tc->GetPosition(), tc->GetRotation(), tc->GetScale());
        tic->state = InterpolationState::TRANSIENT;
    }
}

void NetworkTransformInterpolationSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkTransformInterpolationSystem::ProcessFixed");
    for (TransformInterpolationComponent* tic : fixedInterpolationPendingAdd.components)
    {
        tic->state = InterpolationState::FIXED;
    }
    fixedInterpolationPendingAdd.components.clear();
}

} //namespace DAVA
