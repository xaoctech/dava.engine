#pragma once

#include "Base/Array.h"
#include "Math/Quaternion.h"
#include "Physics/VehicleComponent.h"

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

private:
    void ResetInputData();

    Component* Clone(Entity* toEntity) override;

    DAVA_VIRTUAL_REFLECTION(VehicleCarComponent, VehicleComponent);

private:
    friend class PhysicsVehiclesSubsystem;

    float32 analogAcceleration = 0.0f;
    float32 analogSteer = 0.0f;
    float32 analogBrake = 0.0f;
};
}
