#pragma once

#include <Scene3D/Systems/BaseSimulationSystem.h>
#include "NetworkCore/Scene3D/Systems/SnapshotSystemBase.h"

namespace DAVA
{
class SnapshotSystemClient final : public SnapshotSystemBase, public ISimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(SnapshotSystemClient, SnapshotSystemBase);

    using SnapshotSystemBase::SnapshotSystemBase;

protected:
    bool isResimulation = false;

    bool NeedToBeTracked(Entity* entity) override;
    bool NeedToBeTracked(Component* component) override;
    void RegisterEntity(Entity* entity) override;
    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;
    void ProcessFixed(float32 timeElapsed) override;

    void ReSimulationStart(Entity* entity, uint32 frameId) override;
    void ReSimulationEnd(Entity* entity) override;
    void Simulate(Entity* entity) override;
    const ComponentMask& GetResimulationComponents() const override;
};
}
