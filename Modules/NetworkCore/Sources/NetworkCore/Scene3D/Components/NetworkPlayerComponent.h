#pragma once

#include <Base/FixedVector.h>
#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>
#include "NetworkCore/NetworkTypes.h"

namespace DAVA
{
class NetworkPlayerComponent : public Component
{
    DAVA_VIRTUAL_REFLECTION(NetworkPlayerComponent, Component);

public:
    struct SendPeriodInfo
    {
        const Entity* target;
        uint8 period;
        bool fresh = true;
    };

    NetworkPlayerComponent();

    Component* Clone(Entity* toEntity) override;

    void SetSendPeriod(const Entity* target, uint8 period);

    Vector<SendPeriodInfo> periods;
    FixedVector<NetworkID> visibleEntityIds;

private:
    UnorderedMap<const Entity*, size_t> entityToInternalIndex;
};
}
