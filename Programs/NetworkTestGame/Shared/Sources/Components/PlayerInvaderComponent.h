#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class PlayerInvaderComponent : public DAVA::Component
{
protected:
    virtual ~PlayerInvaderComponent();

public:
    DAVA_VIRTUAL_REFLECTION(PlayerInvaderComponent, DAVA::Component);
    PlayerInvaderComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
};
