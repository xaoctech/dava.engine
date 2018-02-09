#pragma once

#include "NetworkCore/Snapshot.h"
#include "NetworkCore/SnapshotUtils.h"

#include <Base/Set.h>
#include <Base/Vector.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
class NetworkEntitiesSingleComponent;
class NetworkDeltaSingleComponent;
class NetworkReplicationSingleComponent;
class NetworkTimeSingleComponent;
class SnapshotSingleComponent;
class NetworkReplicationSystem2 : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkReplicationSystem2, SceneSystem);

    NetworkReplicationSystem2(Scene* scene);

    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override;

private:
    void ApplyDiffCallback(uint32 frameId, SnapshotApplyParam& param);
    void ApplySnapshotWithoutPrediction(Entity* entity, Snapshot* snapshot);
    void UpdateReplicationInfo(NetworkID entityId, uint32 frameId, bool isChanged);

    struct PendingEntityParams
    {
        Entity* entity;
        Entity* parent;
    };

    struct PendingComponentParams
    {
        Entity* entity;
        uint32 componentIndex;
    };

    Vector<PendingEntityParams> pendingAddEntityOrdered;
    UnorderedMap<Component*, PendingComponentParams> pendingAddComponent;

    SnapshotSingleComponent* snapshotSingleComponent;
    NetworkEntitiesSingleComponent* networkEntities;
    NetworkDeltaSingleComponent* networkDeltaSingleComponent;
    NetworkReplicationSingleComponent* networkReplicationSingleComponent;
    NetworkTimeSingleComponent* networkTimeSingleComponent;

    void AddPendingEntity(Entity* entity, Entity* parent);
    void RemovePendingEntity(Entity* entity);
    Entity* GetPendingEntityParent(Entity* entity);

    void AddPendingComponent(Component* component, Entity* entity, uint32 componentIndex);
    void RemovePendingComponent(const Type* componentType);
    void RemovePendingComponents(Entity* entity);
};
} // namespace DAVA
