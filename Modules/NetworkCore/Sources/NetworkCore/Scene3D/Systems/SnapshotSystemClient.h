#pragma once

#include <Scene3D/Systems/BaseSimulationSystem.h>
#include "NetworkCore/Scene3D/Systems/SnapshotSystemBase.h"

namespace DAVA
{
class SnapshotSystemClient final : public SnapshotSystemBase, public ISimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(SnapshotSystemClient, SnapshotSystemBase);

    SnapshotSystemClient(Scene* scene);

protected:
    bool isResimulation = false;

    bool NeedToBeTracked(Entity* entity) override;
    bool NeedToBeTracked(Component* component) override;
    void RegisterEntity(Entity* entity) override;
    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;
    void ProcessFixed(float32 timeElapsed) override;

    void ReSimulationStart() override;
    void ReSimulationEnd() override;

    void ReSnapEntities();
    const ComponentMask& GetResimulationComponents() const override;

private:
    EntityGroup* entities = nullptr;
};
}
