#pragma once

#include <Base/BaseTypes.h>
#include <Scene3D/ComponentGroup.h>
#include <Scene3D/EntityGroup.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

namespace DAVA
{
class TransformInterpolationComponent;
class NetworkTransformInterpolationSystem : public BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkTransformInterpolationSystem, BaseSimulationSystem);
    NetworkTransformInterpolationSystem(Scene* scene);

    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override{};

    void ReSimulationStart() override;
    void ReSimulationEnd() override;

private:
    ComponentGroup<TransformInterpolationComponent>* fixedInterpolationGroup = nullptr;
    ComponentGroupOnAdd<TransformInterpolationComponent>* fixedInterpolationPendingAdd = nullptr;
};

} //namespace DAVA
