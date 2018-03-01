#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

using namespace DAVA;

class ShootIfSeeingTargetTaskComponent : public BotTaskComponent
{
public:
    ShootIfSeeingTargetTaskComponent() = default;
    explicit ShootIfSeeingTargetTaskComponent(uint32 targetID_);
    Component* Clone(Entity* toEntity) override;

private:
    uint32 targetID;
    bool noAmmo = false;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(ShootIfSeeingTargetTaskComponent, BotTaskComponent, Component);
};
