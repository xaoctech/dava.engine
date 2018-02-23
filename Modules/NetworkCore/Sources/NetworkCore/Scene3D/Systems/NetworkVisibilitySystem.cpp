#include "NetworkVisibilitySystem.h"

#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkVisibilitySingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>

#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkVisibilitySystem)
{
    ReflectionRegistrator<NetworkVisibilitySystem>::Begin()[M::Tags("network", "server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkVisibilitySystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 6.0f)]
    .End();
}

NetworkVisibilitySystem::NetworkVisibilitySystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkReplicationComponent>() | ComponentUtils::MakeMask<NetworkPlayerComponent>())
{
    netVisSingleComp = scene->GetSingletonComponent<NetworkVisibilitySingleComponent>();
}

void NetworkVisibilitySystem::AddEntity(Entity* entity)
{
    playerEntities.insert(entity);
}

void NetworkVisibilitySystem::RemoveEntity(Entity* entity)
{
    playerEntities.erase(entity);
}

void NetworkVisibilitySystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkVisibilitySystem::ProcessFixed");
    for (const Entity* playerEntity : playerEntities)
    {
        const NetworkReplicationComponent* replicationComponent = playerEntity->GetComponent<NetworkReplicationComponent>();
        const NetworkPlayerID playerID = replicationComponent->GetNetworkPlayerID();

        NetworkPlayerComponent* playerComponent = playerEntity->GetComponent<NetworkPlayerComponent>();
        const Vector<NetworkID>& visibleNetworkIDs = netVisSingleComp->GetVisibleNetworkIDs(playerID);
        DVASSERT(visibleNetworkIDs.size() <= MAX_NETWORK_VISIBLE_ENTITIES_COUNT);

        playerComponent->visibleEntityIds.resize(visibleNetworkIDs.size());
        memcpy(playerComponent->visibleEntityIds.data(), visibleNetworkIDs.data(), visibleNetworkIDs.size() * sizeof(uint32));
    }
}

} //namespace DAVA
