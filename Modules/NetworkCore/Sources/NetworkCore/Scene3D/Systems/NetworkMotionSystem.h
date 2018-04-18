#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

namespace DAVA
{
class EntityGroup;

/** System responsible for synchronizing animations. */
class NetworkMotionSystem final : public BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkMotionSystem, SceneSystem);

    NetworkMotionSystem(Scene* scene);

    void ProcessFixedBeginFrame(float32 timeElapsed);
    void ProcessFixedEndFrame(float32 timeElapsed);

    void PrepareForRemove() override;

private:
    void TransferDataToNetworkComponents();
    void TransferDataFromNetworkComponents();

private:
    EntityGroup* networkAnimatedEntities;
};
} //namespace DAVA
