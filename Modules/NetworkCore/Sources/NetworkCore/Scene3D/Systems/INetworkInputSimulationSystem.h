#pragma once

#include "Base/FastName.h"
#include "Base/UnordererSet.h"
#include "Math/Quaternion.h"
#include "Math/Vector.h"
#include "Scene3D/Systems/BaseSimulationSystem.h"
#include "Scene3D/Components/SingleComponents/ActionsSingleComponent.h"

namespace DAVA
{
class Entity;
class Scene;

uint32 GetKeyboardDeviceId();
uint32 GetMouseDeviceId();

class INetworkInputSimulationSystem : public BaseSimulationSystem
{
    DAVA_VIRTUAL_REFLECTION(INetworkInputSimulationSystem, BaseSimulationSystem);

public:
    INetworkInputSimulationSystem(Scene* scene, const ComponentMask& requiredComponents);

    void ReSimulationStart(Entity* entity, uint32 frameId) override;
    void ReSimulationEnd(Entity* entity) override;
    void Simulate(Entity* entity) override;

    virtual void ApplyDigitalActions(Entity* entity,
                                     const Vector<FastName>& actions,
                                     uint32 clientFrameId,
                                     float32 duration) const = 0;

    virtual void ApplyAnalogActions(Entity* entity,
                                    const AnalogActionsMap& actions,
                                    uint32 clientFrameId,
                                    float32 duration) const
    {
    }
};
}
