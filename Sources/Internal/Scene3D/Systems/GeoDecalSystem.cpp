#include "Scene3D/Systems/GeoDecalSystem.h"

namespace DAVA
{
GeoDecalSystem::GeoDecalSystem(Scene* scene)
    : SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
}

void GeoDecalSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_SWITCH_SYSTEM);

    for (auto& decal : decals)
    {
        for (auto& comp : decal.second.lastValidConfig)
        {
            GeoDecalComponent* geoDecalComponent = static_cast<GeoDecalComponent*>(comp.first);
            const GeoDecalComponent::Config& currentConfig = geoDecalComponent->GetConfig();
            if (currentConfig != comp.second)
            {
                BuildDecal(decal.first, geoDecalComponent);
                comp.second = currentConfig;
            }
        }
    }
}

void GeoDecalSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (event == EventSystem::WORLD_TRANSFORM_CHANGED)
    {
        auto it = decals.find(component->GetEntity());
        if (it == decals.end())
            return;

        for (auto& decal : it->second.lastValidConfig)
            decal.second.invalidate();
    }
}

void GeoDecalSystem::AddEntity(Entity* entity)
{
    decals.emplace(entity, entity);
}

void GeoDecalSystem::RemoveEntity(Entity* entity)
{
    for (uint32 i = 0, e = entity->GetComponentCount(Component::GEO_DECAL_COMPONENT); i < e; ++i)
        RemoveComponent(entity, entity->GetComponent(Component::GEO_DECAL_COMPONENT, i));
}

void GeoDecalSystem::AddComponent(Entity* entity, Component* component)
{
    DVASSERT(component->GetType() == Component::GEO_DECAL_COMPONENT);

    auto it = decals.find(entity);
    if (it == decals.end())
    {
        decals.emplace(entity, entity);
    }
    else
    {
        it->second.lastValidConfig[component].invalidate();
    }
}

void GeoDecalSystem::RemoveComponent(Entity* entity, Component* component)
{
    DVASSERT(component->GetType() == Component::GEO_DECAL_COMPONENT);

    auto it = decals.find(entity);
    if (it == decals.end())
        return;

    RemoveCreatedDecals(static_cast<GeoDecalComponent*>(component));
    it->second.lastValidConfig.erase(component);

    // erase entity from map in case it does not contain geo decal components anymore
    if (it->second.lastValidConfig.empty())
        decals.erase(it);
}

void GeoDecalSystem::RemoveCreatedDecals(GeoDecalComponent* component)
{
    Logger::Info("REMOVE BUILT DECALS!");
}

void GeoDecalSystem::BuildDecal(Entity* entity, GeoDecalComponent* component)
{
    Logger::Info("REBUILD DECAL!");
}

/*
 * Geo decal cache implementation
 */
GeoDecalSystem::GeoDecalCache::GeoDecalCache(Entity* entity)
{
    for (uint32 i = 0, e = entity->GetComponentCount(Component::GEO_DECAL_COMPONENT); i < e; ++i)
    {
        Component* component = entity->GetComponent(Component::GEO_DECAL_COMPONENT, i);
        lastValidConfig[component].invalidate();
    }
}
}
