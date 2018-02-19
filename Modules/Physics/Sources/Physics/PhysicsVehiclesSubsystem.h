#pragma once

#include <Base/BaseTypes.h>
#include <Base/Vector.h>
#include <Math/Vector.h>
#include <Scene3D/ComponentGroup.h>

namespace physx
{
class PxScene;
class PxShape;
class PxVehicleWheelsSimData;
class PxBatchQuery;
class PxVehicleDrivableSurfaceToTireFrictionPairs;
}

namespace DAVA
{
class Scene;
class Entity;
class Component;
class CollisionShapeComponent;
class VehicleComponent;
class VehicleCarComponent;
class VehicleTankComponent;
class VehicleChassisComponent;
class VehicleWheelComponent;
class DynamicBodyComponent;

namespace PhysicsVehicleSubsystemDetail
{
class VehicleSceneQueryData;
}

const uint32 GROUND_TYPE = 1 << 0;
const uint32 WHEEL_TYPE = 1 << 1;
const uint32 CHASSIS_TYPE = 1 << 2;

const uint32 GROUND_TYPES_TO_COLLIDE_WITH = CHASSIS_TYPE;
const uint32 WHEEL_TYPES_TO_COLLIDE_WITH = WHEEL_TYPE | CHASSIS_TYPE;
const uint32 CHASSIS_TYPES_TO_COLLIDE_WITH = GROUND_TYPE | WHEEL_TYPE | CHASSIS_TYPE;

class PhysicsVehiclesSubsystem final
{
    friend class PhysicsSystem; // For ProcessFixed and OnSimulationEnabled

public:
    PhysicsVehiclesSubsystem(Scene* scene, physx::PxScene* pxScene);
    ~PhysicsVehiclesSubsystem();

    void SetupNonDrivableSurface(CollisionShapeComponent* surfaceShape) const;
    void SetupDrivableSurface(CollisionShapeComponent* surfaceShape) const;

private:
    void OnCarComponentRemoved(VehicleCarComponent* car);
    void OnTankComponentRemoved(VehicleTankComponent* tank);
    void OnDynamicBodyRemoved(DynamicBodyComponent* body);

    void DeinitVehicleComponent(VehicleComponent* vehicle);

    void ProcessFixed(float32 timeElapsed);
    void OnSimulationEnabled(bool enabled);

    VehicleChassisComponent* GetChassis(VehicleComponent* vehicle) const;
    Vector<VehicleWheelComponent*> GetWheels(VehicleComponent* vehicle) const;
    Vector3 CalculateMomentOfInertiaForShape(CollisionShapeComponent* shape);

    bool TryCreateVehicleCommonParts(VehicleComponent* vehicleComponent, float32 wheelMaxCompression, float32 wheelMaxDroop, float32 wheelSpringStrength, float32 wheelSpringDamperRate, uint32* outWheelsCount, physx::PxVehicleWheelsSimData** outWheelsSimulationData);
    void TryRecreateCarVehicle(VehicleCarComponent* vehicleComponent);
    void TryRecreateTankVehicle(VehicleTankComponent* vehicleComponent);

    void LogVehicleCar(VehicleComponent* vehicleComponent, String name);

private:
    Scene* scene = nullptr;
    physx::PxScene* pxScene = nullptr;
    bool simulationEnabled = false;

    PhysicsVehicleSubsystemDetail::VehicleSceneQueryData* vehicleSceneQueryData = nullptr;
    physx::PxBatchQuery* batchQuery = nullptr;
    physx::PxVehicleDrivableSurfaceToTireFrictionPairs* frictionPairs = nullptr;

    ComponentGroup<VehicleCarComponent>* cars;
    ComponentGroup<VehicleTankComponent>* tanks;
    ComponentGroup<DynamicBodyComponent>* dynamicBodies;
};
}
