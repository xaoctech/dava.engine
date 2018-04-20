#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
class TankAttackTaskComponent final : public BotTaskComponent
{
public:
    TankAttackTaskComponent()
    {
    }
    TankAttackTaskComponent(uint32 targetId, float reloadTime, float initialDelay = 0.f);
    Component* Clone(Entity* toEntity) override;

private:
    uint32 targetId;
    float reloadTime = 1.f;
    float delay = 0.f;
    bool haveTargetPosition = false;
    Vector3 targetPosition;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(TankAttackTaskComponent, BotTaskComponent, Component);
};

class TankMoveToPointTaskComponent final : public BotTaskComponent
{
public:
    TankMoveToPointTaskComponent()
    {
    }
    TankMoveToPointTaskComponent(const Vector3& targetPoint, float precision);
    Component* Clone(Entity* toEntity) override;

private:
    Vector3 targetPoint;
    float precision = 0.5f;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(TankMoveToPointTaskComponent, BotTaskComponent, Component);
};

class TankRandomMovementTaskComponent final : public BotTaskComponent
{
public:
    TankRandomMovementTaskComponent()
    {
    }
    Component* Clone(Entity* toEntity) override;

private:
    bool turnLeft = false;
    bool turnRight = false;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(TankRandomMovementTaskComponent, BotTaskComponent, Component);
};
}