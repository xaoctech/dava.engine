#pragma once

#include <Entity/SceneSystem.h>
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkPredictionSingleComponent.h"

#include <Scene3D/Scene.h>

namespace DAVA
{
struct Snapshot;
struct EntityMisprediction;

class SnapshotBranch;
class SnapshotSingleComponent;
class NetworkEntitiesSingleComponent;
class NetworkReplicationSingleComponent;

class NetworkPredictSystem2 : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkPredictSystem2, SceneSystem);

    NetworkPredictSystem2(Scene* scene);

    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

protected:
    Vector<Entity*> pendingForRemove;

private:
    bool ConfirmComponentValues(Snapshot* clientSnapshot, Snapshot* entitySnapshot, NetworkID entityId, EntityMisprediction& misprediction);

    static const uint32 maxTTL = 32;
    const NetworkReplicationSingleComponent* replicationComponent = nullptr;
    NetworkPredictionSingleComponent* predictionComponent = nullptr;
    SnapshotSingleComponent* snapshotSingleComponent = nullptr;

    struct PredictedEntityInfo
    {
        uint32 ttl = maxTTL;
        uint32 lastExistanceFrameId = 0;
    };

    UnorderedMap<Entity*, PredictedEntityInfo> predictedEntities;
    UnorderedMap<NetworkID, Entity*> predictedEntityIds;
};
} // namespace DAVA
