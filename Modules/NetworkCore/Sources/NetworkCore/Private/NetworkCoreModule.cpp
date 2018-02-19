#include "NetworkCore/NetworkCoreModule.h"

#include "NetworkCore/Compression/CompressorRegistrar.h"

#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkInputComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTrafficLimitComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkRemoteInputComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkDebugDrawComponent.h"

#include "NetworkCore/Scene3D/Systems/INetworkInputSimulationSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkDebugDrawSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkDebugPredictDrawSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemBase.h"
#include "NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemClient.h"
#include "NetworkCore/Scene3D/Systems/NetworkDeltaReplicationSystemServer.h"
#include "NetworkCore/Scene3D/Systems/NetworkGameModeSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkInputSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkPredictSystem2.h"
#include "NetworkCore/Scene3D/Systems/NetworkRemoteInputSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkReplicationSystem2.h"
#include "NetworkCore/Scene3D/Systems/NetworkResimulationSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkTimelineControlSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkTimeSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkTransformFromLocalToNetSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkTransformFromNetToLocalSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkVisibilitySystem.h"
#include "NetworkCore/Scene3D/Systems/SnapshotSystemBase.h"
#include "NetworkCore/Scene3D/Systems/SnapshotSystemClient.h"
#include "NetworkCore/Scene3D/Systems/SnapshotSystemServer.h"
#include "NetworkCore/Scene3D/Systems/NetworkHealthCheckSystem.h"

#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkDeltaSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkEntitiesSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkPredictionSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkReplicationSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkStatisticsSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimelineSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkVisibilitySingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"

#include "NetworkCore/Private/NetworkCoreDebugOverlay.h"
#include "NetworkCore/Scene3D/Systems/NetworkReplicationSystem2.h"
#include "NetworkCore/Snapshot.h"

#include <Engine/Engine.h>
#include <Entity/ComponentManager.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkCoreModule)
{
    ReflectionRegistrator<NetworkCoreModule>::Begin().End();
}

NetworkCoreModule::NetworkCoreModule(Engine* engine)
    : IModule(engine)
{
    // Components
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkDebugDrawComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkTrafficLimitComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkPlayerComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkReplicationComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkPredictComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkInputComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkTransformComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkRemoteInputComponent);

    // SingleComponents
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkClientSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkDeltaSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkEntitiesSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkGameModeSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkPredictionSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkReplicationSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkServerSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkStatisticsSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkTimelineSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkTimeSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkVisibilitySingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SnapshotSingleComponent);

    // Systems
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(INetworkInputSimulationSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkDebugDrawSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkDebugPredictDrawSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkDeltaReplicationSystemBase);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkDeltaReplicationSystemClient);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkDeltaReplicationSystemServer);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkGameModeSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkIdSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkInputSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkPredictSystem2);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkRemoteInputSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkReplicationSystem2);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkResimulationSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkTimelineControlSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkTimeSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkTransformFromLocalToNetSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkTransformFromNetToLocalSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkVisibilitySystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SnapshotSystemBase);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SnapshotSystemClient);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SnapshotSystemServer);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkHealthCheckSystem);

    RegisterStandardTypeCompressors();
    netDebugOverlay.reset(new NetworkCoreDebugOverlayItem());
}

void NetworkCoreModule::Init()
{
    if (nullptr != GetEngineContext()->debugOverlay)
    {
        GetEngineContext()->debugOverlay->RegisterItem(netDebugOverlay.get());
    }
}

void NetworkCoreModule::Shutdown()
{
    if (nullptr != GetEngineContext()->debugOverlay)
    {
        GetEngineContext()->debugOverlay->UnregisterItem(netDebugOverlay.get());
    }
}
}
