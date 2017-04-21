#include "Render/Highlevel/GeoDecalManager.h"

namespace DAVA
{
struct GeoDecalManager::DecalBuildInfo
{
    Mapping mapping = Mapping::PLANAR;
    PolygonGroup* polygonGroup = nullptr;
    NMaterial* material = nullptr;
    AABBox3 box;
    Vector3 projectionAxis;
    Matrix4 projectionSpaceTransform;
    Matrix4 projectionSpaceInverseTransform;
    FilePath image;
};

struct GeoDecalManager::DecalVertex
{
    Vector3 point;
    Vector3 normal;
    Vector3 tangent;
    Vector3 binormal;
    Vector2 texCoord0;
    Vector2 texCoord1;
    Vector4 decalCoord;
    int32 jointCount = 0;
    int32 jointIndex = 0;
    float32 jointWeight = 1.0f;
};

const GeoDecalManager::Decal GeoDecalManager::InvalidDecal = nullptr;

GeoDecalManager::GeoDecalManager(RenderSystem* rs)
    : renderSystem(rs)
{
    uint32 normalmapData[16] = {
        0xffff8080, 0xffff8080, 0xffff8080, 0xffff8080,
        0xffff8080, 0xffff8080, 0xffff8080, 0xffff8080,
        0xffff8080, 0xffff8080, 0xffff8080, 0xffff8080,
        0xffff8080, 0xffff8080, 0xffff8080, 0xffff8080,
    };
    defaultNormalMap = Texture::CreateFromData(PixelFormat::FORMAT_RGBA8888, reinterpret_cast<uint8*>(normalmapData), 4, 4, false);
}

GeoDecalManager::Decal GeoDecalManager::BuildDecal(const DecalConfig& config, const Matrix4& decalWorldTransform, RenderObject* ro)
{
    ++decalCounter;

    uintptr_t thisId = reinterpret_cast<uintptr_t>(this);
    Decal decal = reinterpret_cast<Decal>(decalCounter ^ thisId);
    // todo : use something better for decal id

    AABBox3 worldSpaceBox;
    config.boundingBox.GetTransformedBox(decalWorldTransform, worldSpaceBox);

    Vector3 boxCorners[8];
    config.boundingBox.GetCorners(boxCorners);

    Vector3 dir = MultiplyVectorMat3x3(Vector3(0.0f, 0.0f, -1.0f), decalWorldTransform);
    Vector3 up = MultiplyVectorMat3x3(Vector3(0.0f, -1.0f, 0.0f), decalWorldTransform);
    Vector3 side = MultiplyVectorMat3x3(Vector3(1.0f, 0.0f, 0.0f), decalWorldTransform);

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

    Vector3 boxMin = Vector3(+std::numeric_limits<float>::max(), +std::numeric_limits<float>::max(), +std::numeric_limits<float>::max());
    Vector3 boxMax = Vector3(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (uint32 i = 0; i < 8; ++i)
    {
        Vector3 worldPos = boxCorners[i] * decalWorldTransform;
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

    DecalBuildInfo info;
    info.projectionAxis = dir;
    info.projectionSpaceTransform = view * proj;
    info.projectionSpaceTransform.GetInverse(info.projectionSpaceInverseTransform);
    info.mapping = config.mapping;
    info.image = config.image;

    worldSpaceBox.GetTransformedBox(ro->GetInverseWorldTransform(), info.box);
    BuiltDecal& builtDecal = builtDecals[decal];
    {
        builtDecal.sourceObject = ro;
        builtDecal.renderObject.ConstructInplace();
        builtDecal.renderObject->SetWorldTransformPtr(ro->GetWorldTransformPtr());
        builtDecal.renderObject->SetInverseTransform(ro->GetInverseWorldTransform());
        builtDecal.renderObject->SetLodIndex(ro->GetLodIndex());
        builtDecal.renderObject->SetSwitchIndex(ro->GetSwitchIndex());
        for (uint32 i = 0, e = ro->GetRenderBatchCount(); i < e; ++i)
        {
            int32 lodIndex = -1;
            int32 switchIndex = -1;
            RenderBatch* sourceBatch = ro->GetRenderBatch(i, lodIndex, switchIndex);
            info.polygonGroup = sourceBatch->GetPolygonGroup();
            info.material = sourceBatch->GetMaterial();

            ScopedPtr<RenderBatch> newBatch(new RenderBatch());
            if (BuildDecal(info, newBatch))
            {
                builtDecal.renderObject->AddRenderBatch(newBatch, lodIndex, switchIndex);
            }
        }
    }
    RegisterDecal(decal);

    return decal;
}

void GeoDecalManager::DeleteDecal(Decal decal)
{
    UnregisterDecal(decal);
    builtDecals.erase(decal);
}

void GeoDecalManager::RegisterDecal(Decal decal)
{
    if (builtDecals[decal].renderObject->GetRenderBatchCount() > 0)
    {
        renderSystem->RenderPermanent(builtDecals[decal].renderObject.Get());
        builtDecals[decal].registered = true;
    }
}

void GeoDecalManager::UnregisterDecal(Decal decal)
{
    if (builtDecals[decal].registered)
    {
        renderSystem->RemoveFromRender(builtDecals[decal].renderObject.Get());
        builtDecals[decal].registered = false;
    }
}

GeoDecalManager::Decal GeoDecalManager::GetDecalForRenderObject(RenderObject* ro) const
{
    Decal result = InvalidDecal;
    for (const auto& i : builtDecals)
    {
        if (i.second.sourceObject == ro)
        {
            result = i.first;
            break;
        }
    }
    return result;
}

RenderObject* GeoDecalManager::GetDecalRenderObject(RenderObject* ro) const
{
    RenderObject* result = nullptr;
    for (const auto& i : builtDecals)
    {
        if (i.second.sourceObject == ro)
        {
            result = i.second.renderObject.Get();
            break;
        }
    }
    return result;
}

RenderObject* GeoDecalManager::GetDecalRenderObject(Decal decal) const
{
    auto i = builtDecals.find(decal);
    return (i == builtDecals.end()) ? nullptr : i->second.renderObject.Get();
}

#define MAX_CLIPPED_POLYGON_CAPACITY 9
#define PLANE_THICKNESS_EPSILON 0.00001f

bool GeoDecalManager::BuildDecal(const DecalBuildInfo& info, RenderBatch* dstBatch)
{
    String fxName(info.material->GetEffectiveFXName().c_str());
    std::transform(fxName.begin(), fxName.end(), fxName.begin(), ::tolower);

    if (fxName.find("shadow") != String::npos)
        return false;

    const AABBox3 clipSpaceBox = AABBox3(Vector3(0.0f, 0.0f, 0.0f), 2.0f);
    bool isPlanarProjection = info.mapping == Mapping::PLANAR;

    PolygonGroup* geometry = info.polygonGroup;
    int32 geometryFormat = geometry->GetFormat();

    Vector<DecalVertex> decalGeometry;
    decalGeometry.reserve(geometry->GetIndexCount());

    Set<uint32> triangles;
    geometry->GetGeometryOctTree()->GetTrianglesInBox(info.box, triangles);

    for (uint32 triangleIndex : triangles)
    {
        DecalVertex points[MAX_CLIPPED_POLYGON_CAPACITY] = {};

        for (int32 j = 0; j < 3; ++j)
        {
            int32 idx = 0;
            geometry->GetIndex(3 * triangleIndex + j, idx);
            geometry->GetCoord(idx, points[j].point);

            if (geometryFormat & EVF_TEXCOORD0)
                geometry->GetTexcoord(0, idx, points[j].texCoord0);
            if (geometryFormat & EVF_TEXCOORD1)
                geometry->GetTexcoord(1, idx, points[j].texCoord1);
            if (geometryFormat & EVF_NORMAL)
                geometry->GetNormal(idx, points[j].normal);
            if (geometryFormat & EVF_TANGENT)
                geometry->GetTangent(idx, points[j].tangent);
            if (geometryFormat & EVF_BINORMAL)
                geometry->GetBinormal(idx, points[j].binormal);
            if ((geometryFormat & EVF_JOINTINDEX) || (geometryFormat & EVF_JOINTWEIGHT))
                geometry->GetJointCount(idx, points[j].jointCount);
            if (geometryFormat & EVF_JOINTINDEX)
                geometry->GetJointIndex(idx, points[j].jointIndex);
            if (geometryFormat & EVF_JOINTWEIGHT)
                geometry->GetJointWeight(idx, points[j].jointWeight);
        }

        if (Intersection::BoxTriangle(info.box, points[2].point, points[1].point, points[0].point))
        {
            Vector3 nrm = (points[1].point - points[0].point).CrossProduct(points[2].point - points[0].point);
            nrm.Normalize();

            if ((info.mapping == Mapping::PLANAR) &&
                (nrm.DotProduct(info.projectionAxis) >= -std::numeric_limits<float>::epsilon()))
            {
                // do not place decal on back faces
                continue;
            }

            uint8_t numPoints = 3;
            points[0].point = points[0].point * info.projectionSpaceTransform;
            points[1].point = points[1].point * info.projectionSpaceTransform;
            points[2].point = points[2].point * info.projectionSpaceTransform;

            float minU = 1.0f;
            float maxU = 0.0f;
            for (uint32 i = 0; i < numPoints; ++i)
            {
                if (info.mapping == Mapping::SPHERICAL)
                {
                    Vector3 p = points[i].point;
                    p.Normalize();
                    points[i].decalCoord.x = std::atan2(p.y, p.x);
                    points[i].decalCoord.y = (asin(p.z) + 0.5f * PI) / PI;
                }
                else if (info.mapping == Mapping::CYLINDRICAL)
                {
                    Vector2 c(points[i].point.x, points[i].point.y);
                    c.Normalize();
                    points[i].decalCoord.x = std::atan2(c.y, c.x);
                    points[i].decalCoord.y = points[i].point.z * 0.5f + 0.5f;
                }
                else
                {
                    points[i].decalCoord.x = points[i].point.x * 0.5f + 0.5f;
                    points[i].decalCoord.y = points[i].point.y * 0.5f + 0.5f;
                }
                minU = std::min(minU, points[i].decalCoord.x);
                maxU = std::max(maxU, points[i].decalCoord.x);
            }

            if (!isPlanarProjection)
            {
                bool edgeTriangle = std::abs(minU - maxU) >= PI;
                for (uint32 i = 0; i < numPoints; ++i)
                {
                    float wrap = (edgeTriangle && (points[i].decalCoord.x < 0.0f)) ? 2.0f * PI : 0.0f;
                    points[i].decalCoord.x = (points[i].decalCoord.x + PI + wrap) / (2.0f * PI);
                }
            }

            ClipToBoundingBox(points, &numPoints, clipSpaceBox);

            for (uint32 i = 0; i < numPoints; ++i)
                points[i].point = points[i].point * info.projectionSpaceInverseTransform;

            for (uint32 i = 0; i + 2 < numPoints; ++i)
            {
                decalGeometry.emplace_back(points[0]);
                decalGeometry.emplace_back(points[i + 1]);
                decalGeometry.emplace_back(points[i + 2]);
            }
        }
    }

    if (decalGeometry.empty())
        return false;

    ScopedPtr<PolygonGroup> poly(new PolygonGroup());
    poly->AllocateData(geometryFormat | EVF_DECAL_TEXCOORD, static_cast<uint32>(decalGeometry.size()), static_cast<uint32>(decalGeometry.size()));
    uint32 index = 0;
    for (const DecalVertex& c : decalGeometry)
    {
        poly->SetCoord(index, c.point);
        poly->SetIndex(index, static_cast<int16>(index));
        poly->SetDecalTexcoord(index, c.decalCoord);

        if (geometryFormat & EVF_TEXCOORD0)
            poly->SetTexcoord(0, index, c.texCoord0);

        if (geometryFormat & EVF_TEXCOORD1)
            poly->SetTexcoord(1, index, c.texCoord1);

        if (geometryFormat & EVF_NORMAL)
            poly->SetNormal(index, c.normal);

        if (geometryFormat & EVF_TANGENT)
            poly->SetTangent(index, c.tangent);

        if (geometryFormat & EVF_BINORMAL)
            poly->SetBinormal(index, c.binormal);

        if ((geometryFormat & EVF_JOINTINDEX) || (geometryFormat & EVF_JOINTWEIGHT))
            geometry->SetJointCount(index, c.jointCount);

        if (geometryFormat & EVF_JOINTINDEX)
            geometry->SetJointIndex(index, 0, c.jointIndex);

        if (geometryFormat & EVF_JOINTWEIGHT)
            geometry->SetJointWeight(index, 0, c.jointWeight);

        ++index;
    }
    poly->BuildBuffers();

    ScopedPtr<NMaterial> material(new NMaterial());
    material->SetParent(info.material);
    material->SetFXName(FastName("~res:/Materials/GeoDecal.material"));
    material->SetMaterialName(FastName("GeoDecal"));

    Texture* geoDecalTexture = Texture::CreateFromFile(info.image);
    material->AddTexture(FastName("geodecal"), geoDecalTexture);

    if (fxName.find("lightmap") != String::npos)
    {
        material->AddFlag(FastName("MATERIAL_LIGHTMAP"), 1);
    }

    if (fxName.find("normalizedblinnphong") != String::npos)
    {
        bool perpixel = fxName.find("pervertex") == String::npos;
        material->AddFlag(FastName("NORMALIZED_BLINN_PHONG"), 1);
        material->AddFlag(FastName(perpixel ? "PIXEL_LIT" : "VERTEX_LIT"), 1);
        // material->AddTexture(NMaterialTextureName::TEXTURE_NORMAL, defaultNormalMap);
    }
    material->PreBuildMaterial(PASS_FORWARD);

    dstBatch->SetPolygonGroup(poly);
    dstBatch->SetMaterial(material);
    dstBatch->serializable = false;

    return true;
}

int8_t GeoDecalManager::Classify(int8_t sign, Vector3::eAxis axis, const Vector3& c_v, const DecalVertex& p_v)
{
    const double d = sign * (p_v.point[axis] - c_v[axis]);

    if (d > PLANE_THICKNESS_EPSILON)
        return 1;

    if (d < -PLANE_THICKNESS_EPSILON)
        return -1;

    return 0;
}

void GeoDecalManager::Lerp(float t, const DecalVertex& v1, const DecalVertex& v2, DecalVertex& result)
{
    DVASSERT(v1.jointCount == v2.jointCount);
    DVASSERT(v1.jointIndex == v2.jointIndex);

    result.jointCount = v1.jointCount;
    result.jointIndex = v1.jointIndex;
#define LERP_IMPL(var) result.var = v1.var * (1.0f - t) + v2.var * t;
    LERP_IMPL(point);
    LERP_IMPL(normal);
    LERP_IMPL(tangent);
    LERP_IMPL(binormal);
    LERP_IMPL(texCoord0);
    LERP_IMPL(texCoord1);
    LERP_IMPL(decalCoord);
    LERP_IMPL(jointWeight);
#undef LERP_IMPL
}

void GeoDecalManager::ClipToPlane(DecalVertex* p_vs, uint8_t* nb_p_vs, int8_t sign, Vector3::eAxis axis, const Vector3& c_v)
{
    uint8_t nb = (*nb_p_vs);
    if (nb <= 1)
    {
        *nb_p_vs = 0;
        return;
    }

    DecalVertex new_p_vs[MAX_CLIPPED_POLYGON_CAPACITY];
    uint8_t k = 0;
    bool polygonCompletelyOnPlane = true; // polygon is fully located on clipping plane

    DecalVertex p_v1 = p_vs[nb - 1];
    int8_t d1 = Classify(sign, axis, c_v, p_v1);
    for (uint8_t j = 0; j < nb; ++j)
    {
        const DecalVertex& p_v2 = p_vs[j];
        int8_t d2 = Classify(sign, axis, c_v, p_v2);
        if (d2 < 0)
        {
            polygonCompletelyOnPlane = false;
            if (d1 > 0)
            {
                const float alpha = (p_v2.point[axis] - c_v[axis]) / (p_v2.point[axis] - p_v1.point[axis]);
                Lerp(alpha, p_v2, p_v1, new_p_vs[k]);
                ++k;
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
                Lerp(alpha, p_v2, p_v1, new_p_vs[k]);
                ++k;
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

void GeoDecalManager::ClipToBoundingBox(DecalVertex* p_vs, uint8_t* nb_p_vs, const AABBox3& clipper)
{
    for (uint8_t axis = 0; axis < 3; ++axis)
    {
        ClipToPlane(p_vs, nb_p_vs, 1, static_cast<Vector3::eAxis>(axis), clipper.min);
        ClipToPlane(p_vs, nb_p_vs, -1, static_cast<Vector3::eAxis>(axis), clipper.max);
    }
}

#undef MAX_CLIPPED_POLYGON_CAPACITY
#undef PLANE_THICKNESS_EPSILON

}
