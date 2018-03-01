#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

using namespace DAVA;

class SlideToBorderTaskComponent : public BotTaskComponent
{
public:
    SlideToBorderTaskComponent() = default;
    explicit SlideToBorderTaskComponent(bool movingRight_);
    Component* Clone(Entity* toEntity) override;
    void InverseDirection();

private:
    bool movingRight = false;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(SlideToBorderTaskComponent, BotTaskComponent, Component);
};
