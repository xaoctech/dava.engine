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
        for (auto& comp : decal.second.data)
        {
            GeoDecalComponent* geoDecalComponent = static_cast<GeoDecalComponent*>(comp.first);
            const GeoDecalComponent::Config& currentConfig = geoDecalComponent->GetConfig();
            if (currentConfig != comp.second.lastValidConfig)
            {
                BuildDecal(decal.first, geoDecalComponent);
                comp.second.lastValidConfig = currentConfig;
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

        for (auto& decal : it->second.data)
            decal.second.lastValidConfig.invalidate();
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
        it->second.data[component].lastValidConfig.invalidate();
    }
}

void GeoDecalSystem::RemoveComponent(Entity* entity, Component* component)
{
    DVASSERT(component->GetType() == Component::GEO_DECAL_COMPONENT);

    auto it = decals.find(entity);
    if (it == decals.end())
        return;

    RemoveCreatedDecals(entity, static_cast<GeoDecalComponent*>(component));
    it->second.data.erase(component);

    // erase entity from map in case it does not contain geo decal components anymore
    if (it->second.data.empty())
        decals.erase(it);
}

void GeoDecalSystem::RemoveCreatedDecals(Entity* entity, GeoDecalComponent* component)
{
    GeoDecalCache& decalsForEntity = decals[entity];
    GeoDecalCacheEntry& decalsForComponent = decalsForEntity.data[component];

    for (RefPtr<RenderBatch> batch : decalsForComponent.renderBatches)
        decalsForComponent.renderObject->RemoveRenderBatch(batch.Get());

    decalsForComponent.renderBatches.clear();
    decalsForComponent.renderObject.Set(nullptr);
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
        info.component = component;
        info.entity = entity;
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

        Vector<DecalRenderBatch> builtBatches;
        builtBatches.reserve(ro->GetRenderBatchCount());

        for (uint32 i = 0, e = ro->GetRenderBatchCount(); i < e; ++i)
        {
            info.batch = ro->GetRenderBatch(i, info.lodIndex, info.switchIndex);

            builtBatches.emplace_back();
            if (!BuildDecal(info, builtBatches.back()))
                builtBatches.pop_back();
        }

        decals[entity].data[component].renderObject = ro;
        for (const DecalRenderBatch& info : builtBatches)
        {
            ro->AddRenderBatch(info.batch.Get(), info.lodIndex, info.switchIndex);
            decals[entity].data[component].renderBatches.emplace_back(info.batch);
        }
    }
}

/*
 * Code by Matthias taken from
 * http://codereview.stackexchange.com/questions/131852/high-performance-triangle-axis-aligned-bounding-box-clipping
 */

struct DecalCoord
{
    Vector3 point;
    Vector2 texCoord0;
    Vector2 texCoord1;
};

#define PLANE_THICKNESS_EPSILON 0.00001f
#define MAX_POLYGON_CAPACITY 9

DecalCoord Lerp(float t, const DecalCoord& v1, const DecalCoord& v2)
{
    DecalCoord result;
    result.point = v1.point * (1.0f - t) + v2.point * t;
    result.texCoord0 = v1.texCoord0 * (1.0f - t) + v2.texCoord0 * t;
    result.texCoord1 = v1.texCoord1 * (1.0f - t) + v2.texCoord1 * t;
    return result;
}

inline int8_t Classify(int8_t sign, Vector3::eAxis axis, const Vector3& c_v, const DecalCoord& p_v)
{
    const double d = sign * (p_v.point[axis] - c_v[axis]);

    if (d > PLANE_THICKNESS_EPSILON)
        return 1;

    if (d < -PLANE_THICKNESS_EPSILON)
        return -1;

    return 0;
}

inline void Clip3D_plane(DecalCoord* p_vs, uint8_t* nb_p_vs, int8_t sign, Vector3::eAxis axis, const Vector3& c_v)
{
    uint8_t nb = (*nb_p_vs);
    if (nb == 0)
        return;
    else if (nb == 1)
    {
        *nb_p_vs = 0;
        return;
    }

    DecalCoord new_p_vs[MAX_POLYGON_CAPACITY];
    uint8_t k = 0;
    bool polygonCompletelyOnPlane = true; // polygon is fully located on clipping plane

    DecalCoord p_v1 = p_vs[nb - 1];
    int8_t d1 = Classify(sign, axis, c_v, p_v1);
    for (uint8_t j = 0; j < nb; ++j)
    {
        const DecalCoord& p_v2 = p_vs[j];
        int8_t d2 = Classify(sign, axis, c_v, p_v2);
        if (d2 < 0)
        {
            polygonCompletelyOnPlane = false;
            if (d1 > 0)
            {
                const float alpha = (p_v2.point[axis] - c_v[axis]) / (p_v2.point[axis] - p_v1.point[axis]);
                new_p_vs[k++] = Lerp(alpha, p_v2, p_v1);
            }
            else if (d1 == 0 && (k == 0 || new_p_vs[k - 1].point != p_v1.point))
            {
                new_p_vs[k++] = p_v1;
            }
        }
        else if (d2 > 0)
        {
            polygonCompletelyOnPlane = false;
            if (d1 < 0)
            {
                const float alpha = (p_v2.point[axis] - c_v[axis]) / (p_v2.point[axis] - p_v1.point[axis]);
                new_p_vs[k++] = Lerp(alpha, p_v2, p_v1);
            }
            else if (d1 == 0 && (k == 0 || new_p_vs[k - 1].point != p_v1.point))
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

inline void Clip3D_AABB(DecalCoord* p_vs, uint8_t* nb_p_vs, const AABBox3& clipper)
{
    for (uint8_t axis = 0; axis < 3; ++axis)
    {
        Clip3D_plane(p_vs, nb_p_vs, 1, static_cast<Vector3::eAxis>(axis), clipper.min);
        Clip3D_plane(p_vs, nb_p_vs, -1, static_cast<Vector3::eAxis>(axis), clipper.max);
    }
}

bool GeoDecalSystem::BuildDecal(const DecalBuildInfo& info, DecalRenderBatch& batch)
{
    const AABBox3 clipSpaceBox = AABBox3(Vector3(0.0f, 0.0f, 0.0f), 2.0f);

    PolygonGroup* geometry = info.batch->GetPolygonGroup();
    GeometryOctTree* octree = geometry->GetGeometryOctTree();
    octree->CleanDebugTriangles();

    Vector<DecalCoord> decalGeometry;
    decalGeometry.reserve(geometry->GetIndexCount());

    int32 triangleCount = geometry->GetIndexCount() / 3;
    for (int32 i = 0; i < triangleCount; ++i)
    {
        DecalCoord points[MAX_POLYGON_CAPACITY];

        for (int32 j = 0; j < 3; ++j)
        {
            int32 idx = 0;
            geometry->GetIndex(3 * i + j, idx);

            if (geometry->GetFormat() & EVF_TEXCOORD1)
                geometry->GetTexcoord(1, 3 * i + j, points[j].texCoord1);

            geometry->GetCoord(idx, points[j].point);
        }

        if (Intersection::BoxTriangle(info.box, points[2].point, points[1].point, points[0].point))
        {
            points[0].point = points[0].point * info.projectionSpaceTransform;
            points[1].point = points[1].point * info.projectionSpaceTransform;
            points[2].point = points[2].point * info.projectionSpaceTransform;

            points[0].point.z = std::max(-1.0f, points[0].point.z - 0.0333333f);
            points[1].point.z = std::max(-1.0f, points[1].point.z - 0.0333333f);
            points[2].point.z = std::max(-1.0f, points[2].point.z - 0.0333333f);

            Vector3 nrm = (points[1].point - points[0].point).CrossProduct(points[2].point - points[0].point);
            if (nrm.z < -std::numeric_limits<float>::epsilon())
            {
                uint8_t numPoints = 3;
                Clip3D_AABB(points, &numPoints, clipSpaceBox);

                for (uint32 i = 0; i < numPoints; ++i)
                {
                    points[i].texCoord0.x = points[i].point.x * 0.5f + 0.5f;
                    points[i].texCoord0.y = points[i].point.y * 0.5f + 0.5f;
                    points[i].point = points[i].point * info.projectionSpaceInverseTransform;
                }
                for (uint32 i = 0; i + 2 < numPoints; ++i)
                {
                    decalGeometry.emplace_back(points[0]);
                    decalGeometry.emplace_back(points[i + 1]);
                    decalGeometry.emplace_back(points[i + 2]);
                    // octree->AddDebugTriangle(points[0].point, points[i].point, points[i + 1].point);
                }
            }
        }
    }

    if (decalGeometry.empty())
        return false;

    ScopedPtr<PolygonGroup> poly(new PolygonGroup());
    poly->AllocateData(EVF_VERTEX | EVF_TEXCOORD0 | EVF_TEXCOORD1, static_cast<uint32>(decalGeometry.size()), static_cast<uint32>(decalGeometry.size()));
    uint32 index = 0;
    for (const DecalCoord& c : decalGeometry)
    {
        poly->SetCoord(index, c.point);
        poly->SetTexcoord(0, index, c.texCoord0);
        poly->SetTexcoord(1, index, c.texCoord1);
        poly->SetIndex(index, static_cast<int16>(index));
        ++index;
    }
    poly->BuildBuffers();

    ScopedPtr<NMaterial> material(new NMaterial());
    material->SetFXName(FastName("~res:/Materials/Decal.Debug.material"));
    material->SetMaterialName(FastName("Decal.Debug.material"));

    Texture* lightmap = info.batch->GetMaterial()->GetEffectiveTexture(NMaterialTextureName::TEXTURE_LIGHTMAP);
    if (lightmap != nullptr)
        material->AddTexture(NMaterialTextureName::TEXTURE_LIGHTMAP, lightmap);

    GeoDecalComponent* decalComponent = static_cast<GeoDecalComponent*>(info.component);
    Texture* albedo = Texture::CreateFromFile(decalComponent->GetDecalImage()); //info.component->Get info.batch->GetMaterial()->GetEffectiveTexture(NMaterialTextureName::TEXTURE_ALBEDO);
    if (albedo != nullptr)
        material->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, albedo);

    const float32* uvOffset = info.batch->GetMaterial()->GetEffectivePropValue(NMaterialParamName::PARAM_UV_OFFSET);
    if (uvOffset != nullptr)
        material->AddProperty(NMaterialParamName::PARAM_UV_OFFSET, uvOffset, rhi::ShaderProp::Type::TYPE_FLOAT2);

    const float32* uvScale = info.batch->GetMaterial()->GetEffectivePropValue(NMaterialParamName::PARAM_UV_SCALE);
    if (uvScale != nullptr)
        material->AddProperty(NMaterialParamName::PARAM_UV_SCALE, uvScale, rhi::ShaderProp::Type::TYPE_FLOAT2);

    batch.batch = new RenderBatch();
    batch.batch->SetPolygonGroup(poly);
    batch.batch->SetMaterial(material);
    batch.batch->serializable = false;

    batch.lodIndex = info.lodIndex;
    batch.switchIndex = info.switchIndex;

    return true;
}

/*
 * Geo decal cache implementation
 */
GeoDecalSystem::GeoDecalCache::GeoDecalCache(Entity* entity)
{
    for (uint32 i = 0, e = entity->GetComponentCount(Component::GEO_DECAL_COMPONENT); i < e; ++i)
    {
        Component* component = entity->GetComponent(Component::GEO_DECAL_COMPONENT, i);
        data[component].lastValidConfig.invalidate();
    }
}
}
