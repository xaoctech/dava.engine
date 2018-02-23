#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class PlayerTankComponent : public DAVA::Component
{
protected:
    virtual ~PlayerTankComponent();

public:
    DAVA_VIRTUAL_REFLECTION(PlayerTankComponent, DAVA::Component);
    PlayerTankComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
};
