#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
class InvaderSlideToBorderTaskComponent final : public BotTaskComponent
{
public:
    InvaderSlideToBorderTaskComponent() = default;
    explicit InvaderSlideToBorderTaskComponent(bool movingRight_);
    Component* Clone(Entity* toEntity) override;
    void InverseDirection();

private:
    bool movingRight = false;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(InvaderSlideToBorderTaskComponent, BotTaskComponent, Component);
};

class InvaderWagToBorderTaskComponent final : public BotTaskComponent
{
public:
    InvaderWagToBorderTaskComponent() = default;
    explicit InvaderWagToBorderTaskComponent(bool movingRight_);
    Component* Clone(Entity* toEntity) override;
    void InverseDirection();

private:
    bool movingRight = false;
    bool waggingRight = false;
    uint8 framesWithSameWagging = 0;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(InvaderWagToBorderTaskComponent, BotTaskComponent, Component);
};

class InvaderDodgeCenterTaskComponent final : public BotTaskComponent
{
public:
    InvaderDodgeCenterTaskComponent() = default;
    explicit InvaderDodgeCenterTaskComponent(bool movingRight_);
    Component* Clone(Entity* toEntity) override;
    void InverseDirection();

private:
    bool movingRight = false;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(InvaderDodgeCenterTaskComponent, BotTaskComponent, Component);
};

class InvaderShootIfSeeingTargetTaskComponent final : public BotTaskComponent
{
public:
    InvaderShootIfSeeingTargetTaskComponent() = default;
    explicit InvaderShootIfSeeingTargetTaskComponent(uint32 targetId_);
    Component* Clone(Entity* toEntity) override;

private:
    uint32 targetId;
    bool noAmmo = false;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(InvaderShootIfSeeingTargetTaskComponent, BotTaskComponent, Component);
};
}