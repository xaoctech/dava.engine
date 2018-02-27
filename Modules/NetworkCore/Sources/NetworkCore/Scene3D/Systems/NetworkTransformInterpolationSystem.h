#pragma once

#include <Base/BaseTypes.h>
#include <Scene3D/ComponentGroup.h>
#include <Scene3D/EntityGroup.h>
#include <NetworkCore/Scene3D/Systems/INetworkInputSimulationSystem.h>

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

    void ReSimulationStart(Entity* entity, uint32 frameId) override;
    void ReSimulationEnd(Entity* entity) override;
    void Simulate(Entity* entity) override{};

private:
    ComponentGroup<TransformInterpolationComponent>* fixedInterpolationGroup;
    ComponentGroupOnAdd<TransformInterpolationComponent> fixedInterpolationPendingAdd;
};

} //namespace DAVA
