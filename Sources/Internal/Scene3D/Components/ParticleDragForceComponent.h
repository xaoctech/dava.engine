#pragma once

#include "Entity/Component.h"
#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Particles/ParticlePropertyLine.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class Entity;

class ParticleDragForceComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(PARTICLE_DRAG_FORCE_COMPONENT);

    ParticleDragForceComponent() = default;
    ~ParticleDragForceComponent() = default;

    Component* Clone(Entity* toEnity) override;

private:
    RefPtr<PropertyLine<float32>> forceOverLife;
    float32 someFloat = 0.0f;

public:
    DAVA_VIRTUAL_REFLECTION(ParticleDragForceComponent, Component);
};
}