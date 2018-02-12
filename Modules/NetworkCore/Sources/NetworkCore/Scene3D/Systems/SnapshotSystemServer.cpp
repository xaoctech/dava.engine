#include "NetworkCore/Scene3D/Systems/SnapshotSystemServer.h"

#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SnapshotSystemServer)
{
    ReflectionRegistrator<SnapshotSystemServer>::Begin()[M::Tags("server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &SnapshotSystemServer::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 5.0f)]
    .End();
}

void SnapshotSystemServer::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("SnapshotSystemServer::ProcessFixed");

    UpdateSnapshot();

    uint32 curFrameId = timeSingleComponent->GetFrameId();
    Snapshot* s = snapshotSingleComponent->CreateServerSnapshot(curFrameId);

    *s = snapshot;
    s->frameId = curFrameId;
}
} // namespace DAVA
