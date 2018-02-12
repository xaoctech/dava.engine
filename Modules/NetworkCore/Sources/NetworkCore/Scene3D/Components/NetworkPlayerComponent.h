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
    NetworkPlayerComponent();

    Component* Clone(Entity* toEntity) override;

    FixedVector<NetworkID> visibleEntityIds;
};
}
