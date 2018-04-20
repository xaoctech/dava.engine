#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
class ExternalImpulseComponent : public Component
{
protected:
    virtual ~ExternalImpulseComponent() = default;

public:
    DAVA_VIRTUAL_REFLECTION(ExternalImpulseComponent, Component);
    ExternalImpulseComponent() = default;

    Component* Clone(Entity* toEntity) override;

    bool IsZero() const;
    void Reset();

    Vector3 direction = Vector3::Zero;
    Vector3 velocity = Vector3::Zero;
    float32 magnitude = 0.f;
    bool applied = false;
};
}