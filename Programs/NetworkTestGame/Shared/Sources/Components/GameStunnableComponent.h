#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class GameStunnableComponent : public DAVA::Component
{
public:
    DAVA_VIRTUAL_REFLECTION(GameStunnableComponent, DAVA::Component);
    GameStunnableComponent();
    virtual ~GameStunnableComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    void SetCooldown(DAVA::float32 cooldown);
    DAVA::float32 GetCooldown() const;

    bool IsStunned() const;

private:
    DAVA::float32 cooldown = 0.f;
};
