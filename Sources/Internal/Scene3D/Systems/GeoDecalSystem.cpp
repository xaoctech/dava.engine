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

    Vector3 boxCorners[8];
    AABBox3 worldSpaceBox;
    component->GetBoundingBox().GetTransformedBox(entity->GetWorldTransform(), worldSpaceBox);
    component->GetBoundingBox().GetCorners(boxCorners);

    Vector3 dir = MultiplyVectorMat3x3(Vector3(0.0f, 0.0f, -1.0f), entity->GetWorldTransform());
    Vector3 up = MultiplyVectorMat3x3(Vector3(0.0f, -1.0f, 0.0f), entity->GetWorldTransform());
    Vector3 side = MultiplyVectorMat3x3(Vector3(1.0f, 0.0f, 0.0f), entity->GetWorldTransform());

    Vector<RenderObject*> renderObjects;
    rs->GetRenderHierarchy()->GetAllObjectsInBBox(worldSpaceBox, renderObjects);

    for (RenderObject* ro : renderObjects)
    {
        if (ro->GetType() != RenderObject::eType::TYPE_MESH)
            continue;

        dir = MultiplyVectorMat3x3(dir, ro->GetInverseWorldTransform());
        up = MultiplyVectorMat3x3(up, ro->GetInverseWorldTransform());
        side = MultiplyVectorMat3x3(side, ro->GetInverseWorldTransform());

        Matrix4 view = Matrix4::IDENTITY;
        view._data[0][0] = side.x;
        view._data[0][1] = up.x;
        view._data[0][2] = -dir.x;
        view._data[1][0] = side.y;
        view._data[1][1] = up.y;
        view._data[1][2] = -dir.y;
        view._data[2][0] = side.z;
        view._data[2][1] = up.z;
        view._data[2][2] = -dir.z;

        DecalBuildInfo info;
        info.object = ro;

        Vector3 boxMin = Vector3(+std::numeric_limits<float>::max(), +std::numeric_limits<float>::max(), +std::numeric_limits<float>::max());
        Vector3 boxMax = Vector3(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
        for (uint32 i = 0; i < 8; ++i)
        {
            Vector3 worldPos = boxCorners[i] * entity->GetWorldTransform();
            Vector3 objectPos = worldPos * ro->GetInverseWorldTransform();
            Vector3 t = MultiplyVectorMat3x3(objectPos, view);
            boxMin.x = std::min(boxMin.x, t.x);
            boxMin.y = std::min(boxMin.y, t.y);
            boxMin.z = std::min(boxMin.z, t.z);
            boxMax.x = std::max(boxMax.x, t.x);
            boxMax.y = std::max(boxMax.y, t.y);
            boxMax.z = std::max(boxMax.z, t.z);
        }
        Matrix4 proj;
        proj.OrthographicProjectionLH(boxMin.x, boxMax.x, boxMin.y, boxMax.y, boxMin.z, boxMax.z, false);
        info.projectionSpaceTransform = view * proj;
        info.projectionSpaceTransform.GetInverse(info.projectionSpaceInverseTransform);

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
            v[0] = v[0] * info.projectionSpaceTransform;
            v[1] = v[1] * info.projectionSpaceTransform;
            v[2] = v[2] * info.projectionSpaceTransform;
            const float angleTreshold = -0.087156f; // cos(85deg)
            Vector3 nrm = (v[1] - v[0]).CrossProduct(v[2] - v[0]);
            if (nrm.z < angleTreshold)
            {
                /*
                v[0].z *= 0.5f;
                v[1].z *= 0.5f;
                v[2].z *= 0.5f;
                */
                v[0] = v[0] * info.projectionSpaceInverseTransform;
                v[1] = v[1] * info.projectionSpaceInverseTransform;
                v[2] = v[2] * info.projectionSpaceInverseTransform;
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
