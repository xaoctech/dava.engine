#include "Physics/VehicleCarComponent.h"

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
void VehicleCarComponent::SetAnalogAcceleration(float32 value)
{
    DVASSERT(value >= 0.0f && value <= 1.0f);
    analogAcceleration = value;
}

float32 VehicleCarComponent::GetAnalogAcceleration() const
{
    return analogAcceleration;
}

void VehicleCarComponent::SetAnalogSteer(float32 value)
{
    DVASSERT(value >= -1.0f && value <= 1.0f);
    analogSteer = value;
}

float32 VehicleCarComponent::GetAnalogSteer() const
{
    return analogSteer;
}

void VehicleCarComponent::SetAnalogBrake(float32 value)
{
    DVASSERT(value >= 0.0f && value <= 1.0f);
    analogBrake = value;
}

float32 VehicleCarComponent::GetAnalogBrake() const
{
    return analogBrake;
}

void VehicleCarComponent::ResetInputData()
{
    analogAcceleration = 0.0f;
    analogSteer = 0.0f;
    analogBrake = 0.0f;
}

Component* VehicleCarComponent::Clone(Entity* toEntity)
{
    VehicleCarComponent* result = new VehicleCarComponent();
    result->analogAcceleration = analogAcceleration;
    result->analogSteer = analogSteer;
    result->analogBrake = analogBrake;
    result->SetEntity(toEntity);

    return result;
}

DAVA_VIRTUAL_REFLECTION_IMPL(VehicleCarComponent)
{
    ReflectionRegistrator<VehicleCarComponent>::Begin()
    .ConstructorByPointer()
    .End();
}
}
