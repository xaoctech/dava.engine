#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

using namespace DAVA;

class RandomMovementTaskComponent : public BotTaskComponent
{
public:
    RandomMovementTaskComponent()
    {
    }
    Component* Clone(Entity* toEntity) override;

private:
    bool turnLeft = false;
    bool turnRight = false;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(RandomMovementTaskComponent, BotTaskComponent, Component);
};
