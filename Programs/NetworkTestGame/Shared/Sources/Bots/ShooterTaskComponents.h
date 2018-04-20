#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
class ShooterAttackStandingStillTaskComponent final : public BotTaskComponent
{
public:
    ShooterAttackStandingStillTaskComponent() = default;
    explicit ShooterAttackStandingStillTaskComponent(uint32 targetId);
    Component* Clone(Entity* toEntity) override;

private:
    uint32 targetId;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(ShooterAttackStandingStillTaskComponent, BotTaskComponent, Component);
};

class ShooterAttackPursuingTargetTaskComponent final : public BotTaskComponent
{
public:
    ShooterAttackPursuingTargetTaskComponent() = default;
    explicit ShooterAttackPursuingTargetTaskComponent(uint32 targetId);
    Component* Clone(Entity* toEntity) override;

private:
    uint32 targetId;
    bool forward = true;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(ShooterAttackPursuingTargetTaskComponent, BotTaskComponent, Component);
};

class ShooterAttackCirclingAroundTaskComponent final : public BotTaskComponent
{
public:
    ShooterAttackCirclingAroundTaskComponent() = default;
    ShooterAttackCirclingAroundTaskComponent(uint32 targetId, bool right);
    Component* Clone(Entity* toEntity) override;

private:
    uint32 targetId;
    bool right = true;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(ShooterAttackCirclingAroundTaskComponent, BotTaskComponent, Component);
};

class ShooterAttackWaggingTaskComponent final : public BotTaskComponent
{
public:
    ShooterAttackWaggingTaskComponent() = default;
    explicit ShooterAttackWaggingTaskComponent(uint32 targetId);
    Component* Clone(Entity* toEntity) override;

private:
    uint32 targetId;
    float32 sameDirectionTime = 0.f;
    bool right = true;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(ShooterAttackWaggingTaskComponent, BotTaskComponent, Component);
};

class ShooterMoveToPointShortestTaskComponent final : public BotTaskComponent
{
public:
    ShooterMoveToPointShortestTaskComponent() = default;
    explicit ShooterMoveToPointShortestTaskComponent(const Vector3& point);
    Component* Clone(Entity* toEntity) override;

private:
    Vector3 point;
    float32 accelerateTime = 0.f;
    bool accelerate = false;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(ShooterMoveToPointShortestTaskComponent, BotTaskComponent, Component);
};

class ShooterMoveToPointWindingTaskComponent final : public BotTaskComponent
{
public:
    ShooterMoveToPointWindingTaskComponent() = default;
    explicit ShooterMoveToPointWindingTaskComponent(const Vector3& point);
    Component* Clone(Entity* toEntity) override;

private:
    Vector3 point;
    float32 sameDirectionTime = 0.f;
    bool right = true;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(ShooterMoveToPointWindingTaskComponent, BotTaskComponent, Component);
};

class ShooterHangAroundTaskComponent final : public BotTaskComponent
{
public:
    ShooterHangAroundTaskComponent() = default;
    explicit ShooterHangAroundTaskComponent(Vector3& point, float32 angle);
    Component* Clone(Entity* toEntity) override;

private:
    Vector3 point;
    float32 angle;
    float32 sameDirectionTime = 0.f;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(ShooterHangAroundTaskComponent, BotTaskComponent, Component);
};

class ShooterDriveTaskComponent final : public BotTaskComponent
{
public:
    ShooterDriveTaskComponent() = default;
    ShooterDriveTaskComponent(const Vector3& point, uint32 carId);
    Component* Clone(Entity* toEntity) override;

private:
    Vector3 point;
    float32 actionTime = 0.f;
    uint32 carId;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(ShooterDriveTaskComponent, BotTaskComponent, Component);
};
}
