#pragma once

#include "Entity/SceneSystem.h"
#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
class ISimulationSystem
{
public:
    virtual const ComponentMask& GetResimulationComponents() const = 0;
    virtual void ReSimulationStart(Entity* entity, uint32 frameId) = 0;
    virtual void Simulate(Entity* entity) = 0;
    virtual void ReSimulationEnd(Entity* entity) = 0;
};

class BaseSimulationSystem : public SceneSystem, public ISimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(BaseSimulationSystem, SceneSystem);

    BaseSimulationSystem(Scene* scene, const ComponentMask& requiredComponents);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    const ComponentMask& GetResimulationComponents() const override;
    void ReSimulationStart(Entity* entity, uint32 frameId) override;
    void ReSimulationEnd(Entity* entity) override;

protected:
    Vector<Entity*> entities;
};

} //namespace DAVA
