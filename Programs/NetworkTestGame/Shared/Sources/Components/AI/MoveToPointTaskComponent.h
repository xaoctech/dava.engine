#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

using namespace DAVA;

class MoveToPointTaskComponent : public BotTaskComponent
{
public:
    MoveToPointTaskComponent()
    {
    }
    MoveToPointTaskComponent(const Vector3& targetPoint, float precision);
    Component* Clone(Entity* toEntity) override;

private:
    Vector3 targetPoint;
    float precision = 0.5f;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(MoveToPointTaskComponent, BotTaskComponent, Component);
};
