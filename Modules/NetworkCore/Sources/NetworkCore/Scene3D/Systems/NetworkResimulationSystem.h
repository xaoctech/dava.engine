#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
class SnapshotSingleComponent;
class NetworkPredictionSingleComponent;
class NetworkTimeSingleComponent;
class NetworkResimulationSingleComponent;

class NetworkResimulationSystem : public SceneSystem
{
    DAVA_VIRTUAL_REFLECTION(NetworkResimulationSystem, SceneSystem);

public:
    NetworkResimulationSystem(Scene* scene);
    ~NetworkResimulationSystem();

    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override;

    uint32 GetResimulationsCount() const;

private:
    void OnSystemAdded(SceneSystem* system);
    void OnSystemRemoved(SceneSystem* system);

    UnorderedMap<const Type*, SceneSystem*> resimulationSystems;
    uint32 resimulationsCount = 0;

    const NetworkTimeSingleComponent* networkTimeSingleComponent = nullptr;
    const NetworkPredictionSingleComponent* predictionSingleComponent = nullptr;
    SnapshotSingleComponent* snapshotSingleComponent = nullptr;
    NetworkResimulationSingleComponent* networkResimulationSingleComponent = nullptr;
};
}
