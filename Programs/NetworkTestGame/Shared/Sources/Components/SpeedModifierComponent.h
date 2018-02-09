#pragma once

#include "Entity/Component.h"

using namespace DAVA;

class SpeedModifierComponent : public Component
{
public:
    DAVA_VIRTUAL_REFLECTION(SpeedModifierComponent, Component);

    SpeedModifierComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    void SetFactor(float32 factor);
    float32 GetFactor() const;

protected:
    float32 factor = 1.f;
};

inline void SpeedModifierComponent::SetFactor(float32 factor_)
{
    factor = factor_;
}

inline float32 SpeedModifierComponent::GetFactor() const
{
    return factor;
}
