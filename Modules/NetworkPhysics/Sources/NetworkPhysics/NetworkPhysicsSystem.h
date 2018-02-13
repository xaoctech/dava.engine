#pragma once

#include <Base/BaseTypes.h>
#include <Base/UnordererSet.h>
#include <Base/UnordererMap.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Math/Vector.h>

namespace DAVA
{
class DynamicBodyComponent;
class VehicleCarComponent;
class NetworkDynamicBodyComponent;

/**
    System which is responsible for:
        - Transfering data from physics components to network physics components (e.g. from VehicleCar to NetworkVehicleCar) (server only)
        - Transfering data from network physics components to physics components (client only, before resimulation)
        - Running resimulations
*/
class NetworkPhysicsSystem final : public BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkPhysicsSystem, BaseSimulationSystem);

    NetworkPhysicsSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void ProcessFixed(float32 dt) override;
    void PrepareForRemove() override;
    void ReSimulationStart(DAVA::Entity* entity, uint32 frameId) override;
    void ReSimulationEnd(Entity* entity) override;
    void Simulate(Entity* entity) override;

    /** Logs all the data about a vehicle, for debugging purposes. */
    void LogVehicleCar(VehicleCarComponent* carComponent, String const& header);

private:
    void TransferDataToNetworkComponents();
    void TransferDataFromNetworkComponent(NetworkDynamicBodyComponent* networkDynamicBodyComponent);

    void FreezeEverythingExceptEntity(Entity* entity);
    void UnfreezeEverything();

private:
    UnorderedSet<NetworkDynamicBodyComponent*> networkDynamicBodyComponents;
    UnorderedMap<DynamicBodyComponent*, std::tuple<Vector3, Vector3>> frozenDynamicBodiesParams;
    ComponentGroup<DynamicBodyComponent>* dynamicBodies = nullptr;
};
}
