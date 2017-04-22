#include "Scene3D/Systems/GeoDecalSystem.h"
#include "Render/Highlevel/GeometryOctTree.h"

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
        GeoDecalComponent* geoDecalComponent = static_cast<GeoDecalComponent*>(decal.first);
        const GeoDecalManager::DecalConfig& currentConfig = geoDecalComponent->GetConfig();
        if (currentConfig != decal.second.lastValidConfig)
        {
            Entity* entity = decal.first->GetEntity();
            RemoveCreatedDecals(entity, geoDecalComponent);
            BuildDecal(entity, geoDecalComponent);
            decal.second.lastValidConfig = currentConfig;
        }
    }
}

void GeoDecalSystem::ImmediateEvent(Component* transformComponent, uint32 event)
{
    if (event == EventSystem::WORLD_TRANSFORM_CHANGED)
    {
        Entity* entity = transformComponent->GetEntity();
        for (uint32 i = 0, e = entity->GetComponentCount(Component::GEO_DECAL_COMPONENT); i < e; ++i)
        {
            Component* component = entity->GetComponent(Component::GEO_DECAL_COMPONENT, i);
            decals[component].lastValidConfig.invalidate();
        }
    }
}

void GeoDecalSystem::AddComponent(Entity* entity, Component* component)
{
    DVASSERT(component != nullptr);
    DVASSERT(component->GetType() == Component::GEO_DECAL_COMPONENT);
    DVASSERT(decals.count(component) == 0);

    decals[component].lastValidConfig.invalidate();
}

void GeoDecalSystem::RemoveComponent(Entity* entity, Component* component)
{
    DVASSERT(component != nullptr);
    DVASSERT(component->GetType() == Component::GEO_DECAL_COMPONENT);
    DVASSERT(decals.count(component) > 0);

    RemoveCreatedDecals(entity, static_cast<GeoDecalComponent*>(component));
    decals.erase(component);
}

void GeoDecalSystem::AddEntity(Entity* entity)
{
    for (uint32 i = 0, e = entity->GetComponentCount(Component::GEO_DECAL_COMPONENT); i < e; ++i)
    {
        Component* component = entity->GetComponent(Component::GEO_DECAL_COMPONENT, i);
        AddComponent(entity, component);
    }
}

void GeoDecalSystem::RemoveEntity(Entity* entity)
{
    for (uint32 i = 0, e = entity->GetComponentCount(Component::GEO_DECAL_COMPONENT); i < e; ++i)
    {
        Component* component = entity->GetComponent(Component::GEO_DECAL_COMPONENT, i);
        RemoveComponent(entity, component);
    }
}

void GeoDecalSystem::GatherRenderableEntitiesInBox(Entity* top, const AABBox3& box, Vector<RenderableEntity>& entities)
{
    RenderObject* object = GetRenderObject(top);
    if ((object != nullptr) && (object->GetType() == RenderObject::eType::TYPE_MESH) && object->GetWorldBoundingBox().IntersectsWithBox(box))
        entities.emplace_back(top, object);

    for (int32 i = 0; i < top->GetChildrenCount(); ++i)
        GatherRenderableEntitiesInBox(top->GetChild(i), box, entities);
}

void GeoDecalSystem::RemoveCreatedDecals(Entity* entity, GeoDecalComponent* component)
{
    GeoDecalManager* manager = GetScene()->GetRenderSystem()->GetGeoDecalManager();
    for (GeoDecalManager::Decal decal : decals[component].decals)
    {
        manager->DeleteDecal(decal);
    }
}

void GeoDecalSystem::BuildDecal(Entity* entityWithDecal, GeoDecalComponent* component)
{
    AABBox3 worldSpaceBox;
    component->GetBoundingBox().GetTransformedBox(entityWithDecal->GetWorldTransform(), worldSpaceBox);

    Vector<RenderableEntity> entities;
    GatherRenderableEntitiesInBox(entityWithDecal->GetParent(), worldSpaceBox, entities);

    RenderSystem* renderSystem = GetScene()->GetRenderSystem();
    GeoDecalManager* manager = renderSystem->GetGeoDecalManager();
    for (const RenderableEntity& e : entities)
    {
        GeoDecalManager::Decal decal = manager->BuildDecal(component->GetConfig(), entityWithDecal->GetWorldTransform(), e.renderObject);
        renderSystem->UpdateNearestLights(manager->GetDecalRenderObject(decal));
        decals[component].decals.emplace_back(decal);
    }
}
}
