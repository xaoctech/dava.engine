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

    RemoveCreatedDecals(entity, static_cast<GeoDecalComponent*>(component));
    it->second.lastValidConfig.erase(component);

    // erase entity from map in case it does not contain geo decal components anymore
    if (it->second.lastValidConfig.empty())
        decals.erase(it);
}

void GeoDecalSystem::RemoveCreatedDecals(Entity* entity, GeoDecalComponent* component)
{
    Logger::Info("REMOVE BUILT DECALS!");
}

void GeoDecalSystem::BuildDecal(Entity* entity, GeoDecalComponent* component)
{
    RemoveCreatedDecals(entity, component);

    RenderSystem* rs = GetScene()->GetRenderSystem();

    AABBox3 worldSpaceBox;
    component->GetBoundingBox().GetTransformedBox(entity->GetWorldTransform(), worldSpaceBox);
    Vector3 worldSpaceBoxProjectionAxis = MultiplyVectorMat3x3(Vector3(0.0f, 0.0f, -1.0f), entity->GetWorldTransform());

    Vector<RenderObject*> renderObjects;
    rs->GetRenderHierarchy()->GetAllObjectsInBBox(worldSpaceBox, renderObjects);

    for (RenderObject* ro : renderObjects)
    {
        if (ro->GetType() != RenderObject::eType::TYPE_MESH)
            continue;

        DecalBuildInfo info;
        info.object = ro;
        info.projectionAxis = MultiplyVectorMat3x3(worldSpaceBoxProjectionAxis, ro->GetInverseWorldTransform());
        worldSpaceBox.GetTransformedBox(ro->GetInverseWorldTransform(), info.box);

        for (uint32 i = 0, e = ro->GetRenderBatchCount(); i < e; ++i)
        {
            info.batch = ro->GetRenderBatch(i, info.lodIndex, info.switchIndex);
            BuildDecal(info);
        }
    }
}

void GeoDecalSystem::BuildDecal(const DecalBuildInfo& info)
{
    PolygonGroup* geometry = info.batch->GetPolygonGroup();
    GeometryOctTree* octree = geometry->GetGeometryOctTree();
    octree->CleanDebugTriangles();

    int32 triangleCount = geometry->GetIndexCount() / 3;
    for (int32 i = 0; i < triangleCount; ++i)
    {
        Vector3 v[3];
        for (int32 j = 0; j < 3; ++j)
        {
            int32 idx = 0;
            geometry->GetIndex(3 * i + j, idx);
            geometry->GetCoord(idx, v[j]);
        }

        if (Intersection::BoxTriangle(info.box, v[2], v[1], v[0]))
        {
            Vector3 nrm = (v[1] - v[0]).CrossProduct(v[2] - v[0]);
            if (nrm.DotProduct(info.projectionAxis) < 0.0f)
            {
                octree->AddDebugTriangle(v[0], v[1], v[2]);
            }
        }
    }
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
