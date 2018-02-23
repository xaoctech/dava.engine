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
    ReflectionRegistrator<NetworkTransformFromLocalToNetSystem>::Begin()[M::Tags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkTransformFromLocalToNetSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 3.0f)]
    .End();
}

NetworkTransformFromLocalToNetSystem::NetworkTransformFromLocalToNetSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkTransformComponent>())
{
}

void NetworkTransformFromLocalToNetSystem::AddEntity(Entity* entity)
{
    entities.insert(entity);
}

void NetworkTransformFromLocalToNetSystem::RemoveEntity(Entity* entity)
{
    entities.erase(entity);
}

void NetworkTransformFromLocalToNetSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkTransformFromLocalToNetSystem::ProcessFixed");

    for (auto& entity : entities)
    {
        CopyFromLocalToNet(entity);
    }
}

void NetworkTransformFromLocalToNetSystem::PrepareForRemove()
{
}

void NetworkTransformFromLocalToNetSystem::ReSimulationStart(Entity* entity, uint32 frameId)
{
}

void NetworkTransformFromLocalToNetSystem::ReSimulationEnd(Entity* entity)
{
}

const ComponentMask& NetworkTransformFromLocalToNetSystem::GetResimulationComponents() const
{
    return GetRequiredComponents();
}

void NetworkTransformFromLocalToNetSystem::Simulate(Entity* entity)
{
    CopyFromLocalToNet(entity);
}

void NetworkTransformFromLocalToNetSystem::CopyFromLocalToNet(Entity* entity)
{
    NetworkTransformComponent* netTransformComp = entity->GetComponent<NetworkTransformComponent>();
    TransformComponent* transformComp = entity->GetComponent<TransformComponent>();
    netTransformComp->SetPosition(transformComp->GetPosition());
    netTransformComp->SetOrientation(transformComp->GetRotation());
}

} // namespace DAVA
