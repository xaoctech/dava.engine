#pragma once

#include <Entity/Component.h>
#include <Math/Vector.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
}

// Represents aim orientation (current + final)
// Described using two angles: around right vector and around up vector
// Aim is always positioned at SHOOTER_AIM_OFFSET relative to the character and rotates around it's right axis
class ShooterAimComponent final : public DAVA::Component
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterAimComponent, DAVA::Component);
    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    void SetFinalAngleX(DAVA::float32 value);
    DAVA::float32 GetFinalAngleX() const;

    void SetFinalAngleZ(DAVA::float32 value);
    DAVA::float32 GetFinalAngleZ() const;

    void SetCurrentAngleX(DAVA::float32 value);
    DAVA::float32 GetCurrentAngleX() const;

    void SetCurrentAngleZ(DAVA::float32 value);
    DAVA::float32 GetCurrentAngleZ() const;

private:
    DAVA::float32 finalAngleX = 0.0f;
    DAVA::float32 finalAngleZ = 0.0f;
    DAVA::float32 currentAngleX = 0.0f;
    DAVA::float32 currentAngleZ = 0.0f;
};
