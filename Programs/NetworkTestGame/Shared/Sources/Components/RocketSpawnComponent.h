#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class RocketSpawnComponent : public DAVA::Component
{
public:
    static const DAVA::uint32 THRESHOLD = 100;
    DAVA_VIRTUAL_REFLECTION(RocketSpawnComponent, DAVA::Component);

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
    DAVA::uint32 progress = 0;
};
