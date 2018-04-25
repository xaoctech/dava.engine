#include "NetworkCore/Scene3D/Systems/NetworkTransformFromLocalToNetSystem.h"

#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/TransformComponent.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkTransformFromLocalToNetSystem)
{
    ReflectionRegistrator<NetworkTransformFromLocalToNetSystem>::Begin()[M::SystemTags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkTransformFromLocalToNetSystem::ProcessFixed)[M::SystemProcessInfo(SPI::Group::EngineBegin, SPI::Type::Fixed, 6.0f)]
    .End();
}

NetworkTransformFromLocalToNetSystem::NetworkTransformFromLocalToNetSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<NetworkTransformComponent>())
{
    entities = scene->AquireEntityGroup<NetworkTransformComponent>();
}

void NetworkTransformFromLocalToNetSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkTransformFromLocalToNetSystem::ProcessFixed");

    for (Entity* entity : entities->GetEntities())
    {
        CopyFromLocalToNet(entity);
    }
}

void NetworkTransformFromLocalToNetSystem::PrepareForRemove()
{
}

void NetworkTransformFromLocalToNetSystem::ReSimulationStart()
{
}

void NetworkTransformFromLocalToNetSystem::ReSimulationEnd()
{
}

void NetworkTransformFromLocalToNetSystem::CopyFromLocalToNet(Entity* entity)
{
    NetworkTransformComponent* netTransformComp = entity->GetComponent<NetworkTransformComponent>();
    TransformComponent* transformComp = entity->GetComponent<TransformComponent>();
    const Transform& localTransform = transformComp->GetLocalTransform();
    netTransformComp->SetPosition(localTransform.GetTranslation());
    netTransformComp->SetOrientation(localTransform.GetRotation());
}

} // namespace DAVA
