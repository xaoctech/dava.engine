#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>

namespace DAVA
{
class BehaviorComponent : public Component
{
public:
    BehaviorComponent() = default;
    virtual ~BehaviorComponent() = default;
    Component* Clone(Entity* toEntity) override;

    float32 lifetime = 0.f;
    bool isActor = false;

private:
    friend class BehaviorSystem;

    BotTaskComponent* currentTask = nullptr;

    DAVA_VIRTUAL_REFLECTION(BehaviorComponent, Component);
};
}
