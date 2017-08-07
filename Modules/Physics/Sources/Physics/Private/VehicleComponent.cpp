#include "Physics/VehicleComponent.h"

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
void VehicleComponent::ResetInputData()
{
    digitalAcceleration = false;
    digitalSteerLeft = false;
    digitalSteerRight = false;
    digitalBrake = false;
    analogAcceleration = 0.0f;
    analogSteer = 0.0f;
    analogBrake = 0.0f;
}

void VehicleComponent::SetDigitalAcceleration(bool value)
{
    digitalAcceleration = value;
}

void VehicleComponent::SetDigitalSteerLeft(bool value)
{
    digitalSteerLeft = value;
}

void VehicleComponent::SetDigitalSteerRight(bool value)
{
    digitalSteerRight = value;
}

void VehicleComponent::SetDigitalBrake(bool value)
{
    digitalBrake = value;
}

void VehicleComponent::SetAnalogAcceleration(float32 value)
{
    DVASSERT(value >= 0.0f && value <= 1.0f);
    analogAcceleration = value;
}

void VehicleComponent::SetAnalogSteer(float32 value)
{
    DVASSERT(value >= -1.0f && value <= 1.0f);
    analogSteer = value;
}

void VehicleComponent::SetAnalogBrake(float32 value)
{
    DVASSERT(value >= 0.0f && value <= 1.0f);
    analogBrake = value;
}

physx::PxVehicleDriveNWRawInputData VehicleComponent::GetRawInputData()
{
    physx::PxVehicleDriveNWRawInputData input;
    input.setDigitalAccel(digitalAcceleration);
    input.setDigitalSteerLeft(digitalSteerLeft);
    input.setDigitalSteerRight(digitalSteerRight);
    input.setDigitalBrake(digitalBrake);
    input.setAnalogAccel(analogAcceleration);
    input.setAnalogSteer(analogSteer);
    input.setAnalogBrake(analogBrake);

    return input;
}

uint32 VehicleComponent::GetType() const
{
    return Component::VEHICLE_COMPONENT;
}

Component* VehicleComponent::Clone(Entity* toEntity)
{
    VehicleComponent* result = new VehicleComponent();
    result->SetEntity(toEntity);

    return result;
}

DAVA_VIRTUAL_REFLECTION_IMPL(VehicleComponent)
{
    ReflectionRegistrator<VehicleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}
}