#pragma once

#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

#include <Entity/SceneSystem.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Functional/Signal.h>

class RECommandNotificationObject;

class SceneTreeSystem : public DAVA::SceneSystem, public EditorSceneSystem
{
public:
    SceneTreeSystem(DAVA::Scene* scene);

    void RegisterEntity(DAVA::Entity* entity) override;
    void RegisterComponent(DAVA::Entity* entity, DAVA::Component* component) override;

    void UnregisterEntity(DAVA::Entity* entity) override;
    void UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component) override;
    void PrepareForRemove();

    void Process(DAVA::float32 timeElapsed) override;
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

    struct SyncSnapshot
    {
        DAVA::Map<DAVA::uint32, DAVA::Vector<Selectable>> objectsToRefetch;
        DAVA::Map<DAVA::uint32, DAVA::Vector<Selectable>, std::greater<DAVA::uint32>> removedObjects;

        DAVA::UnorderedSet<Selectable> changedObjects;

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
    return objectsToRefetch.empty() == true && removedObjects.empty() == true && changedObjects.empty() == true;
}
