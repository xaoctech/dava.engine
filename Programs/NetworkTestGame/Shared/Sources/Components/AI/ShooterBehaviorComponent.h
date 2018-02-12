#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

using namespace DAVA;

class ShooterBehaviorComponent : public Component
{
public:
    ShooterBehaviorComponent()
    {
    }
    ShooterBehaviorComponent(bool isActor);
    Component* Clone(Entity* toEntity) override;

private:
    BotTaskComponent* currentTask = nullptr;
    bool isActor = false;

    friend class ShooterBehaviorSystem;

    DAVA_VIRTUAL_REFLECTION(ShooterBehaviorComponent, Component);
};
