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

/*
 * Code by Matthias taken from
 * http://codereview.stackexchange.com/questions/131852/high-performance-triangle-axis-aligned-bounding-box-clipping
 */
#define PLANE_THICKNESS_EPSILON 0.00001f
#define MAX_POLYGON_CAPACITY 9

Vector3 Lerp(float t, const Vector3& v1, const Vector3& v2)
{
    return v1 * (1.0f - t) + v2 * t;
}

template <typename T>
inline int8_t Classify(int8_t sign, Vector3::eAxis axis, const Vector3& c_v, const Vector3& p_v)
{
    const double d = sign * (p_v[axis] - c_v[axis]);

    if (d > PLANE_THICKNESS_EPSILON)
        return 1;

    if (d < -PLANE_THICKNESS_EPSILON)
        return -1;

    return 0;
}

template <typename T>
inline void Clip3D_plane(Vector3* p_vs, uint8_t* nb_p_vs, int8_t sign, Vector3::eAxis axis, const Vector3& c_v)
{
    uint8_t nb = (*nb_p_vs);
    if (nb == 0)
        return;
    else if (nb == 1)
    {
        *nb_p_vs = 0;
        return;
    }

    Vector3 new_p_vs[MAX_POLYGON_CAPACITY];
    uint8_t k = 0;
    bool polygonCompletelyOnPlane = true; // polygon is fully located on clipping plane

    Vector3 p_v1 = p_vs[nb - 1];
    int8_t d1 = Classify<T>(sign, axis, c_v, p_v1);
    for (uint8_t j = 0; j < nb; ++j)
    {
        const Vector3& p_v2 = p_vs[j];
        int8_t d2 = Classify<T>(sign, axis, c_v, p_v2);
        if (d2 < 0)
        {
            polygonCompletelyOnPlane = false;
            if (d1 > 0)
            {
                const float alpha = (p_v2[axis] - c_v[axis]) / (p_v2[axis] - p_v1[axis]);
                new_p_vs[k++] = Lerp(alpha, p_v2, p_v1);
            }
            else if (d1 == 0 && (k == 0 || new_p_vs[k - 1] != p_v1))
            {
                new_p_vs[k++] = p_v1;
            }
        }
        else if (d2 > 0)
        {
            polygonCompletelyOnPlane = false;
            if (d1 < 0)
            {
                const float alpha = (p_v2[axis] - c_v[axis]) / (p_v2[axis] - p_v1[axis]);
                new_p_vs[k++] = Lerp(alpha, p_v2, p_v1);
            }
            else if (d1 == 0 && (k == 0 || new_p_vs[k - 1] != p_v1))
            {
                new_p_vs[k++] = p_v1;
            }
            new_p_vs[k++] = p_v2;
        }
        else
        {
            if (d1 != 0)
                new_p_vs[k++] = p_v2;
        }

        p_v1 = p_v2;
        d1 = d2;
    }

    if (polygonCompletelyOnPlane)
        return;

    *nb_p_vs = k;
    for (uint8_t j = 0; j < k; ++j)
        p_vs[j] = new_p_vs[j];
}

inline void Clip3D_AABB(Vector3* p_vs, uint8_t* nb_p_vs, const AABBox3& clipper)
{
    for (uint8_t axis = 0; axis < 3; ++axis)
    {
        Clip3D_plane<float>(p_vs, nb_p_vs, 1, static_cast<Vector3::eAxis>(axis), clipper.min);
        Clip3D_plane<float>(p_vs, nb_p_vs, -1, static_cast<Vector3::eAxis>(axis), clipper.max);
    }
}

void GeoDecalSystem::BuildDecal(const DecalBuildInfo& info)
{
    PolygonGroup* geometry = info.batch->GetPolygonGroup();
    GeometryOctTree* octree = geometry->GetGeometryOctTree();
    octree->CleanDebugTriangles();

    auto clip = [octree, &info](const Vector3& v1, const Vector3& v2, const Vector3& v3) {
        Vector3 points[MAX_POLYGON_CAPACITY] = { v1, v2, v3 };
        uint8_t numPoints = 3;
        Clip3D_AABB(points, &numPoints, AABBox3(Vector3(0, 0, 0), 2.0f));

        for (uint32 i = 0; i < numPoints; ++i)
            points[i] = points[i] * info.projectionSpaceInverseTransform;

        for (uint32 i = 0; i + 1 < numPoints; ++i)
            octree->AddDebugTriangle(points[0], points[i], points[i + 1]);
    };

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
            Vector3 nrm = (v[1] - v[0]).CrossProduct(v[2] - v[0]);
            if (nrm.z < -std::numeric_limits<float>::epsilon())
            {
                clip(v[0], v[1], v[2]);
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
