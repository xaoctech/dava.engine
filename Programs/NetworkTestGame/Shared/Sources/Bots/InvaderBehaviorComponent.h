#pragma once

#include "BehaviorComponent.h"
#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class InvaderBehaviorSystem;

class InvaderBehaviorComponent final : public BehaviorComponent
{
public:
    enum Role : uint8
    {
        OBSERVER = 0,
        SHOOTER,
        TARGET,

        ROLES_COUNT
    };

    enum Scenario : uint8
    {
        STILL = 0, // trivial
        SLIDING_TARGET,
        SLIDING_SHOOTER,
        SLIDING_BOTH,
        WAGGING_TARGET,
        WAGGING_SHOOTER,
        WAGGING_BOTH,
        DODGING_TARGET,

        SCENARIOS_COUNT
    };

    InvaderBehaviorComponent() = default;
    Component* Clone(Entity* toEntity) override;

    void SetRole(Role value);
    Role GetRole() const;

    Scenario GetScenario() const;

    uint16 GetSyncCounter() const;

    static const Vector<String> roleNames;
    static const Vector<String> scenarioNames;

private:
    friend class InvaderBehaviorSystem;

    Role role = OBSERVER;

    uint8 currentScenario = STILL;
    float32 timeBeforeScenarioSwitch = 0;
    uint16 syncCounter = 0;

    DAVA_VIRTUAL_REFLECTION(InvaderBehaviorComponent, BehaviorComponent);
};

inline void InvaderBehaviorComponent::SetRole(InvaderBehaviorComponent::Role value)
{
    role = value;
}

inline InvaderBehaviorComponent::Role InvaderBehaviorComponent::GetRole() const
{
    return role;
}

inline InvaderBehaviorComponent::Scenario InvaderBehaviorComponent::GetScenario() const
{
    return static_cast<Scenario>(currentScenario);
}

inline uint16 InvaderBehaviorComponent::GetSyncCounter() const
{
    return syncCounter;
}
}
