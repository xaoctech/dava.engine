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
    .Method("ProcessFixed", &NetworkTransformFromNetToLocalSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 3.0f)]
    .End();
}

NetworkTransformFromNetToLocalSystem::NetworkTransformFromNetToLocalSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkTransformComponent>())
{
}

void NetworkTransformFromNetToLocalSystem::AddEntity(Entity* entity)
{
    entities.insert(entity);
}

void NetworkTransformFromNetToLocalSystem::RemoveEntity(Entity* entity)
{
    entities.erase(entity);
}

void NetworkTransformFromNetToLocalSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkTransformFromNetToLocalSystem::ProcessFixed");

    for (auto& entity : entities)
    {
        CopyFromNetToLocal(entity);
    }
}

void NetworkTransformFromNetToLocalSystem::PrepareForRemove()
{
}

void NetworkTransformFromNetToLocalSystem::ReSimulationStart(Entity* entity, uint32 frameId)
{
}

void NetworkTransformFromNetToLocalSystem::ReSimulationEnd(Entity* entity)
{
}

const ComponentMask& NetworkTransformFromNetToLocalSystem::GetResimulationComponents() const
{
    return GetRequiredComponents();
}

void NetworkTransformFromNetToLocalSystem::Simulate(Entity* entity)
{
    CopyFromNetToLocal(entity);
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
