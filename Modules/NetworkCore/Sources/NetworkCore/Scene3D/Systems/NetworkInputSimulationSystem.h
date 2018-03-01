#pragma once

#include <Base/FastName.h>
#include <Base/UnordererMap.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>

namespace DAVA
{
class Entity;
class Scene;
class NetworkTimeSingleComponent;

class NetworkInputSimulationSystem : public BaseSimulationSystem
{
    DAVA_VIRTUAL_REFLECTION(NetworkInputSimulationSystem, BaseSimulationSystem);

public:
    NetworkInputSimulationSystem(Scene* scene);

    void ReSimulationStart() override;
    void ReSimulationEnd() override;
    void ProcessFixed(float32 dt) override;

private:
    ActionsSingleComponent* asc = nullptr;
    EntityGroup* entityGroup = nullptr;

    const NetworkTimeSingleComponent* networkTimeSingleComponent = nullptr;

    struct
    {
        UnorderedMap<uint8, Vector<ActionsSingleComponent::Actions>> playerIdsToActions;
    } backUp;
};
}
