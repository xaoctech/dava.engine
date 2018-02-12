#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
}

// Responsible for indicating which role an entity has
class ShooterStateComponent final : public DAVA::Component
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterStateComponent, DAVA::Component);
    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    DAVA::uint32 raycastAttackFrameId = 0;
    DAVA::uint32 lastRaycastAttackFrameId = 0;
};