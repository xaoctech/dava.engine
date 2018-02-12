#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class GameStunningComponent : public DAVA::Component
{
protected:
    virtual ~GameStunningComponent();

public:
    DAVA_VIRTUAL_REFLECTION(GameStunningComponent, DAVA::Component);
    GameStunningComponent();
    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
};
