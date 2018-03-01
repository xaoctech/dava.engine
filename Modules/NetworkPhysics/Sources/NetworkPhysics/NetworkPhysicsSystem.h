#pragma once

#include <Base/BaseTypes.h>
#include <Base/UnordererSet.h>
#include <Base/UnordererMap.h>
#include <Math/Vector.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

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

    void ProcessFixed(float32 dt) override;
    void PrepareForRemove() override;
    void ReSimulationStart() override;
    void ReSimulationEnd() override;

    /** Logs all the data about a vehicle, for debugging purposes. */
    void LogVehicleCar(VehicleCarComponent* carComponent, String const& header);

private:
    void TransferDataToNetworkComponents();
    void TransferDataFromNetworkComponent(Entity* entity);

    void FreezeEverything();
    void UnfreezeBody(DynamicBodyComponent* body);
    void UnfreezeEverything();

private:
    EntityGroup* entityGroup = nullptr;
    UnorderedMap<DynamicBodyComponent*, std::tuple<Vector3, Vector3>> frozenDynamicBodiesParams;
    ComponentGroup<DynamicBodyComponent>* dynamicBodies = nullptr;
};
}
