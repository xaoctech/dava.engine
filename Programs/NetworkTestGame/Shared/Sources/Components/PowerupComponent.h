#pragma once

#include "Entity/Component.h"

using namespace DAVA;

enum class PowerupType : uint8
{
    HEALTH,
    SPEED
};

struct PowerupDescriptor
{
    PowerupType type = PowerupType::HEALTH;
    float32 factor = 1.f;
};

class PowerupComponent : public Component
{
public:
    DAVA_VIRTUAL_REFLECTION(PowerupComponent, Component);

    PowerupComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    void SetDescriptor(const PowerupDescriptor& descr);
    const PowerupDescriptor& GetDescriptor() const;

protected:
    PowerupDescriptor descriptor;
};

inline void PowerupComponent::SetDescriptor(const PowerupDescriptor& descriptor_)
{
    descriptor = descriptor_;
}

inline const PowerupDescriptor& PowerupComponent::GetDescriptor() const
{
    return descriptor;
}
