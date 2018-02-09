#pragma once

#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
class ISimulationSystem;
class SnapshotSingleComponent;
class NetworkPredictionSingleComponent;
class NetworkTimeSingleComponent;
class ChangedSystemsSingleComponent;

class NetworkResimulationSystem : public SceneSystem
{
    DAVA_VIRTUAL_REFLECTION(NetworkResimulationSystem, SceneSystem);

public:
    NetworkResimulationSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override;

    void CollectSystems();

    uint32 GetResimulationsCount() const;

private:
    struct ResimulationSystem
    {
        ISimulationSystem* system;
        float32 order;
        SP::Group group;
    };

    Vector<ResimulationSystem> systems;
    bool isCollectedSystems = false;
    uint32 resimulationsCount = 0;

    NetworkTimeSingleComponent* timeComp = nullptr;
    NetworkPredictionSingleComponent* predictionComp = nullptr;
    SnapshotSingleComponent* ssc = nullptr;
    ChangedSystemsSingleComponent* changedSystemsSingleComponent = nullptr;
};
}
