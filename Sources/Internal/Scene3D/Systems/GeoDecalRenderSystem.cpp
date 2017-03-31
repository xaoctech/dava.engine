#include "Scene3D/Systems/GeoDecalRenderSystem.h"

namespace DAVA
{
GeoDecalRenderSystem::GeoDecalRenderSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void GeoDecalRenderSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_SWITCH_SYSTEM);
}

void GeoDecalRenderSystem::ImmediateEvent(Component* component, uint32 event)
{
}

void GeoDecalRenderSystem::AddEntity(Entity* entity)
{
}

void GeoDecalRenderSystem::RemoveEntity(Entity* entity)
{
}

void GeoDecalRenderSystem::AddComponent(Entity* entity, Component* component)
{
}

void GeoDecalRenderSystem::RemoveComponent(Entity* entity, Component* component)
{
}
}
