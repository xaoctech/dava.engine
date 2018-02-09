#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>
#include <enet/enet.h>

namespace DAVA
{
class NetworkTrafficLimitComponent : public Component
{
protected:
    virtual ~NetworkTrafficLimitComponent();

public:
    static const uint32 DEFAULT_ERROR_THRESHOLD = ENET_HOST_DEFAULT_MTU;
    static const uint32 DEFAULT_WARNING_THRESHOLD = static_cast<uint32>(ENET_HOST_DEFAULT_MTU * 0.75f);

    DAVA_VIRTUAL_REFLECTION(NetworkTrafficLimitComponent, Component);
    NetworkTrafficLimitComponent();

    DAVA::Component* Clone(Entity* toEntity) override;

    uint32 errorThreshold = DEFAULT_ERROR_THRESHOLD;
    uint32 warningThreshold = DEFAULT_WARNING_THRESHOLD;
};
}
