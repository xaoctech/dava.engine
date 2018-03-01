#pragma once

#include "BotTaskComponent.h"

#include "Entity/Component.h"
#include <Reflection/Reflection.h>

using namespace DAVA;

class InvaderBehaviorComponent : public Component
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
    explicit InvaderBehaviorComponent(bool isActor_);
    Component* Clone(DAVA::Entity* toEntity) override;

    void SetRole(Role value);
    Role GetRole() const;

    Scenario GetScenario() const;

    uint16 GetSyncCounter() const;

    static const Vector<String> roleNames;
    static const Vector<String> scenarioNames;

private:
    bool isActor = false;
    uint16 syncCounter = 0;

    Role role = OBSERVER;
    uint8 currentScenario = STILL;
    float32 timeBeforeScenarioSwitch = 0;

    BotTaskComponent* currentTask = nullptr;

    friend class InvaderBehaviorSystem;

    DAVA_VIRTUAL_REFLECTION(InvaderBehaviorComponent, Component);
};
