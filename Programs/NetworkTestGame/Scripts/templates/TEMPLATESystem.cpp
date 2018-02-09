#include "TEMPLATESystem.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Logger/Logger.h"

#include <algorithm>

using namespace DAVA;
namespace TEMPLATESystemDetail
{
}

TEMPLATESystem::TEMPLATESystem(DAVA::Scene* scene)
    : BaseSimulationSystem(scene)
{
}

void TEMPLATESystem::AddEntity(Entity* entity)
{
    entities.insert(entity);
}

void TEMPLATESystem::RemoveEntity(Entity* entity)
{
    entities.erase(entity);
}

void TEMPLATESystem::Simulate(DAVA::Entity* entity)
{
}

void TEMPLATESystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    for (Entity* entity : entities)
    {
        Simulate(entity);
    }
}
