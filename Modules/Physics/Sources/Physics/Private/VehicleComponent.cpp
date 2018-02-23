#include "Physics/VehicleComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
physx::PxVehicleDrive* VehicleComponent::GetPxVehicle()
{
    return vehicle;
}

void VehicleComponent::SetGear(eVehicleGears value)
{
    gear = value;
}

eVehicleGears VehicleComponent::GetGear() const
{
    return gear;
}

DAVA_VIRTUAL_REFLECTION_IMPL(VehicleComponent)
{
    ReflectionRegistrator<VehicleComponent>::Begin()
    .Field("Gear", &VehicleComponent::GetGear, &VehicleComponent::SetGear)[M::HiddenField()]
    .End();
}
}
