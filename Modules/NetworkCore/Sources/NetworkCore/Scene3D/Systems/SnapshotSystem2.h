#pragma once

#include <Entity/SceneSystem.h>
#include <Entity/Component.h>
#include <Reflection/Reflection.h>

#include "NetworkCore/Snapshot.h"

namespace DAVA
{
class NetworkTimeSingleComponent;
class NetworkReplicationComponent;
class SnapshotSystem2 : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(SnapshotSystem2, SceneSystem);

    SnapshotSystem2(Scene* scene);
    ~SnapshotSystem2();

    void ProcessFixed(float32 timeElapsed) override;

    void RegisterEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;
    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;
    void RegisterSingleComponent(Component* component) override;
    void UnregisterSingleComponent(Component* component) override;

    void Watch(Entity* entity);
    void Unwatch(Entity* entity);

    void AddPendingEntityReset(Entity* entity);
    void AddPendingEntityEvent(Entity* parent, NetworkID childEntityId, int32 type);
    void AddPendingComponentEvent(Entity* entity, Component* component, int32 type);

    void ResetEntitySnapshot(Entity* entity);
    void AddComponentSnapshot(Entity* entity, Component* component);

    virtual bool NeedToBeTracked(Entity* entity);
    virtual bool NeedToBeTracked(Component* component, NetworkReplicationComponent* nrc = nullptr);

private:
    Vector<Entity*> snapshotedEntities;

    struct PendingData
    {
        bool needReset = false;
        Vector<SnapEntityEvent> entityEvents;
        Vector<SnapComponentEvent> componentEvents;
    };

    UnorderedMap<Entity*, PendingData> pendingEntities;
    const NetworkTimeSingleComponent* timeSingleComponent;

    void UpdateChildrenLayoutVersion(Entity* entity);
    void UpdateChildrenDataVersion(Entity* entity);
};

class SnapshotComponent2 : public Component
{
    DAVA_VIRTUAL_REFLECTION(SnapshotComponent2, Component);

public:
    Component* Clone(Entity* toEntity) override;
    SnapEntity* Get(uint32 frameId, bool create = false);

    struct SnapWatchPoint
    {
        Any lastValue;
        ReflectedObject object;
        const ValueWrapper* vw = nullptr;

        uint32 componentIndex;
        uint32 fieldIndex;
    };

    SnapEntity snapEntity;
    Vector<SnapWatchPoint> watchPoints;

private:
    size_t historyPos = 0;
    Array<SnapEntity, 64> history;
};

} //namespace DAVA
