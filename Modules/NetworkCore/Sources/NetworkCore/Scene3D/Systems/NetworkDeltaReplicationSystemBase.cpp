#include "NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemBase.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include <enet/enet.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkDeltaReplicationSystemBase)
{
    ReflectionRegistrator<NetworkDeltaReplicationSystemBase>::Begin()
    .End();
}

NetworkDeltaReplicationSystemBase::NetworkDeltaReplicationSystemBase(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkReplicationComponent>())
{
}

} //namespace DAVA
