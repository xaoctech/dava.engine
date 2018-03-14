#pragma once

#include "Base/Array.h"
#include "Math/Quaternion.h"
#include "Physics/Vehicles/VehicleComponent.h"
#include "Base/FixedVector.h"

#include <physx/vehicle/PxVehicleSDK.h>
#include <physx/vehicle/PxVehicleDriveNW.h>

namespace DAVA
{
class Entity;

class VehicleCarComponent final : public VehicleComponent
{
public:
    void SetAnalogAcceleration(float32 value);
    float32 GetAnalogAcceleration() const;

    void SetAnalogSteer(float32 value);
    float32 GetAnalogSteer() const;

    void SetAnalogBrake(float32 value);
    float32 GetAnalogBrake() const;

    Component* Clone(Entity* toEntity) override;

    DAVA_VIRTUAL_REFLECTION(VehicleCarComponent, VehicleComponent);

    float32 engineSpeed;
    std::array<float32, physx::PxVehicleDriveNWControl::eMAX_NB_DRIVENW_ANALOG_INPUTS> analogInputStates;

private:
    void ResetInputData();

private:
    friend class PhysicsVehiclesSubsystem;

    float32 analogAcceleration = 0.0f;
    float32 analogSteer = 0.0f;
    float32 analogBrake = 0.0f;
};
}
