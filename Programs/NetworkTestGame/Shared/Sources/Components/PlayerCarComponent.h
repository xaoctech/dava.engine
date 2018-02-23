#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class PlayerCarComponent : public DAVA::Component
{
protected:
    virtual ~PlayerCarComponent();

public:
    DAVA_VIRTUAL_REFLECTION(PlayerTankComponent, DAVA::Component);
    PlayerCarComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
};
