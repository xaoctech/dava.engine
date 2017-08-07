#pragma once

#include <Entity/SceneSystem.h>

namespace physx
{
class PxScene;
}

namespace DAVA
{
class VehicleComponent;
class VehicleChassisComponent;
class VehicleWheelComponent;

class PhysicsVehiclesSubsystem final
{
public:
    PhysicsVehiclesSubsystem(Scene* scene, physx::PxScene* pxScene);
    ~PhysicsVehiclesSubsystem();

    void RegisterEntity(Entity* entity);
    void UnregisterEntity(Entity* entity);
    void RegisterComponent(Entity* entity, Component* component);
    void UnregisterComponent(Entity* entity, Component* component);

    void Simulate(float32 timeElapsed);
    void OnSimulationEnabled(bool enabled);

    void SetupDrivableSurface(physx::PxShape* surfaceShape) const;

private:
    VehicleChassisComponent* GetChassis(VehicleComponent* vehicle) const;
    Vector<VehicleWheelComponent*> GetWheels(VehicleComponent* vehicle) const;
    Vector3 CalculateMomentOfInertiaForShape(CollisionShapeComponent* shape);

    void CreatePhysxVehicle(VehicleComponent* vehicleComponent);

private:
    physx::PxScene* pxScene;

    Vector<VehicleComponent*> vehicleComponents;

    bool simulationEnabled = false;
};
}