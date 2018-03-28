#pragma once

#include "NetworkCore/NetworkTypes.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
class NetworkPlayerComponent;
class NetworkTimeSingleComponent;
class NetworkPredictionSingleComponent;
class NetworkEntitiesSingleComponent;
class SnapshotSingleComponent;
class NetworkResimulationSingleComponent;

class NetworkResimulationSystem : public SceneSystem
{
    DAVA_VIRTUAL_REFLECTION(NetworkResimulationSystem, SceneSystem);

public:
    NetworkResimulationSystem(Scene* scene);
    ~NetworkResimulationSystem();

    void CollectBbHistory(float32 timeElapsed);

    void ProcessFixed(float32 timeElapsed) override;

    uint32 GetMispredictedEntitiesCount() const;
    uint32 GetResimulatedEntitiesCount() const;

private:
    void UpdateListOfResimulatingEntities();
    void OnSystemAdded(SceneSystem* system);
    void OnSystemRemoved(SceneSystem* system);

    EntityGroup* predictictedEntities = nullptr;

    uint32 mispredictedEntitiesCount = 0;
    uint32 resimulatedEntitiesCount = 0;

    UnorderedMap<const Type*, SceneSystem*> resimulationSystems;

    const NetworkTimeSingleComponent* networkTimeSingleComponent = nullptr;
    const NetworkPredictionSingleComponent* predictionSingleComponent = nullptr;
    const NetworkEntitiesSingleComponent* networkEntitiesSingleComponent = nullptr;
    SnapshotSingleComponent* snapshotSingleComponent = nullptr;
    NetworkResimulationSingleComponent* networkResimulationSingleComponent = nullptr;
};
}
