#pragma once

#include "Entity/SceneSystem.h"
#include "Scene3D/EntityGroup.h"

namespace DAVA
{
class ISimulationSystem
{
public:
    virtual const ComponentMask& GetResimulationComponents() const = 0;
    virtual void ReSimulationStart() = 0;
    virtual void ReSimulationEnd() = 0;
};

class BaseSimulationSystem : public SceneSystem, public ISimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(BaseSimulationSystem, SceneSystem);

    BaseSimulationSystem(Scene* scene, const ComponentMask& requiredComponents);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    const ComponentMask& GetResimulationComponents() const override;
    void ReSimulationStart() override;
    void ReSimulationEnd() override;

    static void ReSimulationOn();
    static void ReSimulationOff();
    static bool IsReSimulating();

private:
    static bool isReSimulating;
};

} //namespace DAVA
