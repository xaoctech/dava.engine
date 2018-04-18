#include "NetworkCore/Scene3D/Systems/NetworkTransformFromNetToLocalSystem.h"

#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/NetworkTransformUtils.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/TransformComponent.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkTransformFromNetToLocalSystem)
{
    ReflectionRegistrator<NetworkTransformFromNetToLocalSystem>::Begin()[M::Tags("network", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkTransformFromNetToLocalSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 14.0f)]
    .End();
}

NetworkTransformFromNetToLocalSystem::NetworkTransformFromNetToLocalSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<NetworkTransformComponent>())
{
    entities = scene->AquireEntityGroup<NetworkTransformComponent>();
}

void NetworkTransformFromNetToLocalSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkTransformFromNetToLocalSystem::ProcessFixed");

    for (Entity* entity : entities->GetEntities())
    {
        NetworkTransformComponent* netTransformComp = entity->GetComponent<NetworkTransformComponent>();
        DVASSERT(netTransformComp != nullptr);

        NetworkTransformUtils::CopyToTransform(netTransformComp);
    }
}

void NetworkTransformFromNetToLocalSystem::PrepareForRemove()
{
}
} // namespace DAVA
