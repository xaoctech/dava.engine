#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

using namespace DAVA;

class AttackTaskComponent : public BotTaskComponent
{
public:
    AttackTaskComponent()
    {
    }
    AttackTaskComponent(uint32 targetID, float reloadTime, float initialDelay = 0.f);
    Component* Clone(Entity* toEntity) override;

private:
    uint32 targetID;
    float reloadTime = 1.f;
    float delay = 0.f;
    bool haveTargetPosition = false;
    Vector3 targetPosition;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(AttackTaskComponent, BotTaskComponent, Component);
};
