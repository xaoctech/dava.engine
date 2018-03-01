#include "NetworkCore/Scene3D/Systems/NetworkTransformFromNetToLocalSystem.h"

#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/NetworkCoreUtils.h"

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
        CopyFromNetToLocal(entity);
    }
}

void NetworkTransformFromNetToLocalSystem::PrepareForRemove()
{
}

void NetworkTransformFromNetToLocalSystem::CopyFromNetToLocal(Entity* entity)
{
    NetworkTransformComponent* netTransformComp = entity->GetComponent<NetworkTransformComponent>();
    TransformComponent* transformComp = entity->GetComponent<TransformComponent>();

    if (transformComp->GetPosition() != netTransformComp->GetPosition() ||
        transformComp->GetRotation() != netTransformComp->GetOrientation())
    {
        transformComp->SetLocalTransform(netTransformComp->GetPosition(), netTransformComp->GetOrientation(), transformComp->GetScale());
    }
}
} // namespace DAVA
