#pragma once
#include <Entity/SceneSystem.h>
#include <Scene3D/EntityGroup.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class NetworkMovementSystem : public BaseSimulationSystem
{
    DAVA_VIRTUAL_REFLECTION(NetworkMovementSystem, BaseSimulationSystem);

public:
    NetworkMovementSystem(Scene* scene);
    ~NetworkMovementSystem() = default;

    void Process(float32 timeElapsed) override;
    void ProcessFixed(float32 timeElapsed) override;

    void ReSimulationStart() override;
    void ReSimulationEnd() override;

private:
    bool isResimulation = false;
    uint32 frameOffset = 1;
    uint32 historySize = 7;

    EntityGroup* transformAndMovementGroup = nullptr;

    void ApplyInterpolation();
    void ApplyResimulationSmoothness();

    float32 ApplySmoothFn(float32 value, uint32 smootType);
};

} // namespace DAVA
