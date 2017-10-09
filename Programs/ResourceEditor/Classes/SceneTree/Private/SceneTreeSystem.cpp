#include "Classes/SceneTree/Private/SceneTreeSystem.h"

namespace SceneTreeSystemDetail
{
DAVA::uint32 CalcEntityDepth(DAVA::Entity* e)
{
    DAVA::uint32 depth = 0;
    while (e != nullptr)
    {
        ++depth;
        e = e->GetParent();
    }

    return depth;
}
} // namespace SceneTreeSystemDetail

SceneTreeSystem::SceneTreeSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
}

void SceneTreeSystem::RegisterEntity(DAVA::Entity* entity)
{
    using namespace SceneTreeSystemDetail;
    if (entity == GetScene())
    {
        return;
    }

    GetNextSyncSnapshot().addedEntities[CalcEntityDepth(entity)].push_back(entity);
}

void SceneTreeSystem::RegisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    if (entity == GetScene())
    {
        return;
    }
    GetNextSyncSnapshot().changedEntities.emplace(entity);
}

void SceneTreeSystem::UnregisterEntity(DAVA::Entity* entity)
{
    using namespace SceneTreeSystemDetail;
    if (entity == GetScene())
    {
        return;
    }
    GetNextSyncSnapshot().removedEntities[CalcEntityDepth(entity)].push_back(entity);
}

void SceneTreeSystem::UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    if (entity == GetScene())
    {
        return;
    }
    GetNextSyncSnapshot().changedEntities.emplace(entity);
}

void SceneTreeSystem::PrepareForRemove()
{
    syncSnapshots[0] = syncSnapshots[1] = SyncSnapshot();
}

void SceneTreeSystem::Process(DAVA::float32 timeElapsed)
{
    // if current snapshot is empty, it means that scene tree has already taken this part of diff
    // if next snapshot is not empty, it means that there is a new changes that should be synced with scene tree
    if (syncSnapshots[GetCurrentSyncSnapshotIndex()].IsEmpty() == true &&
        syncSnapshots[GetNextSyncSnapshotIndex()].IsEmpty() == false)
    {
        MoveToNextSnapshot();
        syncIsNecessary.Emit();
    }
}

const SceneTreeSystem::SyncSnapshot& SceneTreeSystem::GetSyncSnapshot() const
{
    return syncSnapshots[GetCurrentSyncSnapshotIndex()];
}

void SceneTreeSystem::SyncFinished()
{
    syncSnapshots[GetCurrentSyncSnapshotIndex()] = SyncSnapshot();
}

SceneTreeSystem::SyncSnapshot& SceneTreeSystem::GetNextSyncSnapshot()
{
    return syncSnapshots[GetNextSyncSnapshotIndex()];
}
