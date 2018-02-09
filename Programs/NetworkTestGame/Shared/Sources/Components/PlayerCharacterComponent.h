#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class PlayerCharacterComponent : public DAVA::Component
{
protected:
    virtual ~PlayerCharacterComponent();

public:
    DAVA_VIRTUAL_REFLECTION(PlayerTankComponent, DAVA::Component);
    PlayerCharacterComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
};
