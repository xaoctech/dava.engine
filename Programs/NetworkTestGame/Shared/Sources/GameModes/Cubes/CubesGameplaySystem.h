#pragma once

#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>

namespace DAVA
{
class Camera;
} // namespace DAVA

class CubesGameplaySystem final : public DAVA::BaseSimulationSystem
{
    DAVA_VIRTUAL_REFLECTION(CubesGameplaySystem, DAVA::BaseSimulationSystem);

public:
    CubesGameplaySystem(DAVA::Scene* scene);
    ~CubesGameplaySystem();

    void ProcessFixed(DAVA::float32 timeElapsed) override;

private:
    void ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration);
    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const;
    void ScheduleMagneteForces(DAVA::Entity* bigCube, DAVA::Entity* smallCube) const;
    void UpdateCameraPosition(DAVA::Entity* bigCube) const;

    DAVA::EntityGroup* bigCubes = nullptr;
    DAVA::EntityGroup* smallCubes = nullptr;

    DAVA::Entity* localCube = nullptr;
    DAVA::Camera* camera = nullptr;
};