#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

using namespace DAVA;

class WaitTaskComponent : public BotTaskComponent
{
public:
    enum class Type : uint8
    {
        DELAY,
        TIMESTAMP
    };

    WaitTaskComponent()
    {
    }
    WaitTaskComponent(Type type, float time);
    Component* Clone(Entity* toEntity) override;

private:
    float time = 0.f;
    Type type = Type::DELAY;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(WaitTaskComponent, BotTaskComponent, Component);
};
