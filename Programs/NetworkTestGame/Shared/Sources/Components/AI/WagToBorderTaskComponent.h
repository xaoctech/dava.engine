#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

using namespace DAVA;

class WagToBorderTaskComponent : public BotTaskComponent
{
public:
    WagToBorderTaskComponent() = default;
    explicit WagToBorderTaskComponent(bool movingRight_);
    Component* Clone(Entity* toEntity) override;
    void InverseDirection();

private:
    bool movingRight = false;
    bool waggingRight = false;
    uint8 framesWithSameWagging = 0;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(WagToBorderTaskComponent, BotTaskComponent, Component);
};
