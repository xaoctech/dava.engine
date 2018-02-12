#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class VTDecalComponent;
class SplineComponent;
class VTSplineDecalSystem : public SceneSystem
{
public:
    VTSplineDecalSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

private:
    void RebuildDecalGeometryData(VTDecalComponent* decalComponent, SplineComponent* splineComponent);
};
}