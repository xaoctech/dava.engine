#include "Scene3D/Systems/BaseSimulationSystem.h"

#include "Debug/DVAssert.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(BaseSimulationSystem)
{
    ReflectionRegistrator<BaseSimulationSystem>::Begin()
    .End();
}

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
    entities.push_back(entity);
}

void BaseSimulationSystem::RemoveEntity(Entity* entity)
{
    uint32 size = static_cast<uint32>(entities.size());
    for (uint32 i = 0; i < size; ++i)
    {
        if (entities[i] == entity)
        {
            entities[i] = entities[size - 1];
            entities.pop_back();
            return;
        }
    }
    DVASSERT(0);
}

void BaseSimulationSystem::ReSimulationStart(Entity* entity, uint32 frameId)
{
}

void BaseSimulationSystem::ReSimulationEnd(Entity* entity)
{
}

} //namespace DAVA
