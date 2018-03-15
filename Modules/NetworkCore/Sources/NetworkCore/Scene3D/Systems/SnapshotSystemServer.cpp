#include "NetworkCore/Scene3D/Systems/SnapshotSystemServer.h"

#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"

#include <Scene3D/Var.h>
#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/SingleComponents/ObservableVarsSingleComponent.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SnapshotSystemServer)
{
    ReflectionRegistrator<SnapshotSystemServer>::Begin()[M::Tags("server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &SnapshotSystemServer::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 8.0f)]
    .End();
}

void SnapshotSystemServer::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("SnapshotSystemServer::ProcessFixed");

    {
        DAVA_PROFILER_CPU_SCOPE("SnapshotSystemServer::ProcessFixed::ObservableVars");
        ObservableVarsSingleComponent* observableVars = GetScene()->GetSingleComponent<ObservableVarsSingleComponent>();
        if (observableVars != nullptr)
        {
            const auto& changes = observableVars->GetChanges();
            for (const auto& change : changes)
            {
                const IVar* var = change.first;
                const Any& value = change.second;
                SnapshotField* snapField = var->GetData().Get<SnapshotField*>();
                DVASSERT(snapField);
                snapField->value = ApplyQuantization(value, snapField->compression, snapField->deltaPrecision);
            }
            observableVars->Clear();
        }
    }

    UpdateSnapshot();

    uint32 curFrameId = timeSingleComponent->GetFrameId();
    Snapshot* s = snapshotSingleComponent->CreateServerSnapshot(curFrameId);

    *s = snapshot;
    s->frameId = curFrameId;
}
} // namespace DAVA
