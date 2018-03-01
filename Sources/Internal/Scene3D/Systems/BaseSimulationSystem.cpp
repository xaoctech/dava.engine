#include "Scene3D/Systems/BaseSimulationSystem.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(BaseSimulationSystem)
{
    ReflectionRegistrator<BaseSimulationSystem>::Begin()
    .End();
}

bool BaseSimulationSystem::isReSimulating = false;

BaseSimulationSystem::BaseSimulationSystem(Scene* scene, const ComponentMask& requiredComponents)
    : SceneSystem(scene, requiredComponents)
{
}

const ComponentMask& BaseSimulationSystem::GetResimulationComponents() const
{
    return GetRequiredComponents();
}

void BaseSimulationSystem::AddEntity(Entity* entity)
{
}

void BaseSimulationSystem::RemoveEntity(Entity* entity)
{
}

void BaseSimulationSystem::ReSimulationStart()
{
}

void BaseSimulationSystem::ReSimulationEnd()
{
}

void BaseSimulationSystem::ReSimulationOn()
{
    DVASSERT(!isReSimulating);

    isReSimulating = true;
}

void BaseSimulationSystem::ReSimulationOff()
{
    DVASSERT(isReSimulating);

    isReSimulating = false;
}

bool BaseSimulationSystem::IsReSimulating()
{
    return isReSimulating;
}

} //namespace DAVA
