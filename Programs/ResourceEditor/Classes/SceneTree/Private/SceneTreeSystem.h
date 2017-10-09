#pragma once

#include <Entity/SceneSystem.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Functional/Signal.h>

#include <functional>

class SceneTreeSystem : public DAVA::SceneSystem
{
public:
    SceneTreeSystem(DAVA::Scene* scene);

    void RegisterEntity(DAVA::Entity* entity) override;
    void RegisterComponent(DAVA::Entity* entity, DAVA::Component* component) override;

    void UnregisterEntity(DAVA::Entity* entity) override;
    void UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component) override;
    void PrepareForRemove();

    void Process(DAVA::float32 timeElapsed) override;

    struct SyncSnapshot
    {
        DAVA::Map<DAVA::uint32, DAVA::Vector<DAVA::Entity*>> addedEntities;
        DAVA::Map<DAVA::uint32, DAVA::Vector<DAVA::Entity*>, std::greater<DAVA::uint32>> removedEntities;

        DAVA::UnorderedSet<DAVA::Entity*> changedEntities;

        bool IsEmpty() const;
    };

    const SyncSnapshot& GetSyncSnapshot() const;
    void SyncFinished();
    DAVA::Signal<> syncIsNecessary;

private:
    SyncSnapshot& GetNextSyncSnapshot();

    DAVA::int32 GetCurrentSyncSnapshotIndex() const;
    DAVA::int32 GetNextSyncSnapshotIndex() const;
    void MoveToNextSnapshot();

    DAVA::Array<SyncSnapshot, 2> syncSnapshots;
    DAVA::int32 currentSnapshot = 0;
};

inline DAVA::int32 SceneTreeSystem::GetCurrentSyncSnapshotIndex() const
{
    return currentSnapshot;
}

inline DAVA::int32 SceneTreeSystem::GetNextSyncSnapshotIndex() const
{
    return ((currentSnapshot + 1) & 0x1);
}

inline void SceneTreeSystem::MoveToNextSnapshot()
{
    currentSnapshot = GetNextSyncSnapshotIndex();
}

inline bool SceneTreeSystem::SyncSnapshot::IsEmpty() const
{
    return addedEntities.empty() == true && removedEntities.empty() == true && changedEntities.empty() == true;
}
