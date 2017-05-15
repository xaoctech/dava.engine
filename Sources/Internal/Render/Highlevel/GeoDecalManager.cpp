#include "Render/Highlevel/GeoDecalManager.h"
#include "Render/Highlevel/SkinnedMesh.h"
#include "Render/Highlevel/GeometryOctTree.h"
#include "Render/Highlevel/RenderObject.h"
#include "Reflection/Reflection.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
struct GeoDecalManager::DecalBuildInfo : public GeoDecalManager::DecalConfig
{
    PolygonGroup* polygonGroup = nullptr;
    NMaterial* material = nullptr;
    Vector4* jointPositions = nullptr;
    Vector4* jointQuaternions = nullptr;
    Vector3 projectionAxis;
    Matrix4 projectionSpaceTransform;
    Matrix4 projectionSpaceInverseTransform;
    int32 jointCount = 0;
    bool useCustomNormal = false;
    bool useSkinning = false;
};

struct GeoDecalManager::DecalVertex
{
    Vector3 originalPoint;
    Vector3 actualPoint;
    Vector3 normal;
    Vector3 tangent;
    Vector3 binormal;
    Vector2 texCoord0;
    Vector2 texCoord1;
    Vector4 decalCoord;
    int32 jointIndex = 0;

    DecalVertex(const DecalVertex& r)
    {
        memcpy(this, &r, sizeof(DecalVertex));
    }

    DecalVertex(DecalVertex&& r)
    {
        memcpy(this, &r, sizeof(DecalVertex));
    }

    DecalVertex& operator=(const DecalVertex& r)
    {
        memcpy(this, &r, sizeof(DecalVertex));
        return *this;
    }

    DecalVertex& operator=(DecalVertex&& r)
    {
        memcpy(this, &r, sizeof(DecalVertex));
        return *this;
    }
};

GeoDecalManager::BuiltDecal::BuiltDecal(const BuiltDecal& r)
    : sourceObject(SafeRetain(r.sourceObject))
    , batchProvider(SafeRetain(r.batchProvider))
{
}

GeoDecalManager::BuiltDecal& GeoDecalManager::BuiltDecal::operator=(const BuiltDecal& r)
{
    SafeRelease(sourceObject);
    sourceObject = SafeRetain(r.sourceObject);

    SafeRelease(batchProvider);
    batchProvider = SafeRetain(r.batchProvider);

    return *this;
}

GeoDecalManager::BuiltDecal::~BuiltDecal()
{
    SafeRelease(sourceObject);
    SafeRelease(batchProvider);
}

class GeoDecalRenderBatchProvider : public RenderBatchProvider
{
public:
    ~GeoDecalRenderBatchProvider()
    {
        for (const RenderBatchWithOptions& b : batches)
        {
            RenderBatch* renderBatch = b.renderBatch;
            SafeRelease(renderBatch);
        }
    }

    const Vector<RenderBatchWithOptions> GetRenderBatches() const
    {
        return batches;
    }

    template <class... A>
    void EmplaceRenderBatch(A&&... args)
    {
        batches.emplace_back(std::forward<A>(args)...);
        SafeRetain(batches.back().renderBatch);
    }

private:
    Vector<RenderBatchWithOptions> batches;
};

const GeoDecalManager::Decal GeoDecalManager::InvalidDecal = nullptr;

GeoDecalManager::GeoDecalManager(RenderSystem* rs)
    : renderSystem(rs)
{
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

    Vector3 dir = MultiplyVectorMat3x3(Vector3(0.0f, 0.0f, 1.0f), decalWorldTransform);
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
    info.albedo = config.albedo;
    info.normal = config.normal;
    info.uvOffset = config.uvOffset;
    info.uvScale = config.uvScale;
    info.useCustomNormal = FileSystem::Instance()->Exists(info.normal);
    info.useSkinning = ro->GetType() == RenderObject::TYPE_SKINNED_MESH;

    if (info.useSkinning)
    {
        SkinnedMesh* mesh = static_cast<SkinnedMesh*>(ro);
        info.jointPositions = mesh->GetPositionArray();
        DVASSERT(info.jointPositions != nullptr);

        info.jointQuaternions = mesh->GetQuaternionArray();
        DVASSERT(info.jointQuaternions != nullptr);

        info.jointCount = mesh->GetJointsArraySize();
    }

    worldSpaceBox.GetTransformedBox(ro->GetInverseWorldTransform(), info.boundingBox);

    BuiltDecal& builtDecal = builtDecals[decal];
    {
        GeoDecalRenderBatchProvider* decalBatchProvider = new GeoDecalRenderBatchProvider();
        builtDecal.batchProvider = decalBatchProvider;
        builtDecal.sourceObject = SafeRetain(ro);

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
                decalBatchProvider->EmplaceRenderBatch(newBatch, lodIndex, switchIndex);
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
    DVASSERT(builtDecals.count(decal) > 0);

    const BuiltDecal& builtDecal = builtDecals[decal];
    builtDecal.sourceObject->AddRenderBatchProvider(builtDecal.batchProvider);
}

void GeoDecalManager::UnregisterDecal(Decal decal)
{
    DVASSERT(builtDecals.count(decal) > 0);

    const BuiltDecal& builtDecal = builtDecals[decal];
    builtDecal.sourceObject->RemoveRenderBatchProvider(builtDecal.batchProvider);
}

void GeoDecalManager::RemoveRenderObject(RenderObject* ro)
{
    for (const auto& b : builtDecals)
    {
        if (b.second.sourceObject == ro)
        {
            UnregisterDecal(b.first);
        }
    }
}

#define MAX_CLIPPED_POLYGON_CAPACITY 9
#define PLANE_THICKNESS_EPSILON 0.00001f

void GeoDecalManager::AddVerticesToGeometry(const DecalBuildInfo& info, DecalVertex* points, Vector<uint8>& buffer)
{
    const AABBox3 clipSpaceBox = AABBox3(Vector3(0.0f, 0.0f, 0.0f), 2.0f);

    uint8_t numPoints = 3;
    points[0].actualPoint = points[0].actualPoint * info.projectionSpaceTransform;
    points[1].actualPoint = points[1].actualPoint * info.projectionSpaceTransform;
    points[2].actualPoint = points[2].actualPoint * info.projectionSpaceTransform;

    float minU = 1.0f;
    float maxU = 0.0f;
    for (uint32 i = 0; i < numPoints; ++i)
    {
        if (info.mapping == Mapping::SPHERICAL)
        {
            Vector3 p = points[i].actualPoint;
            p.Normalize();
            points[i].decalCoord.x = -std::atan2(p.y, p.x);
            points[i].decalCoord.y = (std::asin(p.z) + 0.5f * PI) / PI;
            minU = std::min(minU, points[i].decalCoord.x);
            maxU = std::max(maxU, points[i].decalCoord.x);
        }
        else if (info.mapping == Mapping::CYLINDRICAL)
        {
            points[i].decalCoord.x = -std::atan2(points[i].actualPoint.y, points[i].actualPoint.x);
            points[i].decalCoord.y = points[i].actualPoint.z * 0.5f + 0.5f;
            minU = std::min(minU, points[i].decalCoord.x);
            maxU = std::max(maxU, points[i].decalCoord.x);
        }
        else
        {
            points[i].decalCoord.x = points[i].actualPoint.x * 0.5f + 0.5f;
            points[i].decalCoord.y = points[i].actualPoint.y * 0.5f + 0.5f;
        }
    }

    if (info.mapping != Mapping::PLANAR)
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
    {
        points[i].decalCoord.x = points[i].decalCoord.x * info.uvScale.x + info.uvOffset.x;
        points[i].decalCoord.y = points[i].decalCoord.y * info.uvScale.y + info.uvOffset.y;
    }

    size_t offset = buffer.size();
    buffer.resize(offset + 3 * (numPoints - 2) * sizeof(DecalVertex));
    DecalVertex* decalVertexPtr = reinterpret_cast<DecalVertex*>(buffer.data() + offset);
    for (uint32 i = 0; i + 2 < numPoints; ++i)
    {
        *decalVertexPtr++ = points[0];
        *decalVertexPtr++ = points[i + 1];
        *decalVertexPtr++ = points[i + 2];
    }
}

void GeoDecalManager::GetStaticMeshGeometry(const DecalBuildInfo& info, Vector<uint8>& buffer)
{
    char decalVertexData[MAX_CLIPPED_POLYGON_CAPACITY * sizeof(DecalVertex)];
    DecalVertex* points = reinterpret_cast<DecalVertex*>(decalVertexData);

    Set<uint16> triangles;
    info.polygonGroup->GetGeometryOctTree()->GetTrianglesInBox(info.boundingBox, triangles);

    int32 geometryFormat = info.polygonGroup->GetFormat();

    for (uint16 triangleIndex : triangles)
    {
        int16 idx[3];
        info.polygonGroup->GetTriangleIndices(3 * triangleIndex, idx);
        info.polygonGroup->GetCoord(idx[0], points[0].originalPoint);
        info.polygonGroup->GetCoord(idx[1], points[1].originalPoint);
        info.polygonGroup->GetCoord(idx[2], points[2].originalPoint);
        points[0].actualPoint = points[0].originalPoint;
        points[1].actualPoint = points[1].originalPoint;
        points[2].actualPoint = points[2].originalPoint;

        if (geometryFormat & EVF_TEXCOORD1)
        {
            info.polygonGroup->GetTexcoord(1, idx[0], points[0].texCoord1);
            info.polygonGroup->GetTexcoord(1, idx[1], points[1].texCoord1);
            info.polygonGroup->GetTexcoord(1, idx[2], points[2].texCoord1);
        }
        if (geometryFormat & EVF_NORMAL)
        {
            info.polygonGroup->GetNormal(idx[0], points[0].normal);
            info.polygonGroup->GetNormal(idx[1], points[1].normal);
            info.polygonGroup->GetNormal(idx[2], points[2].normal);
        }
        if (geometryFormat & EVF_TANGENT)
        {
            info.polygonGroup->GetTangent(idx[0], points[0].tangent);
            info.polygonGroup->GetTangent(idx[1], points[1].tangent);
            info.polygonGroup->GetTangent(idx[2], points[2].tangent);
        }
        if (geometryFormat & EVF_BINORMAL)
        {
            info.polygonGroup->GetBinormal(idx[0], points[0].binormal);
            info.polygonGroup->GetBinormal(idx[1], points[1].binormal);
            info.polygonGroup->GetBinormal(idx[2], points[2].binormal);
        }
        Vector3 nrm = (points[1].actualPoint - points[0].actualPoint).CrossProduct(points[2].actualPoint - points[0].actualPoint);
        if ((info.mapping != Mapping::PLANAR) || (nrm.DotProduct(info.projectionAxis) < -std::numeric_limits<float>::epsilon()))
        {
            AddVerticesToGeometry(info, points, buffer);
        }
    }
}

void GeoDecalManager::GetSkinnedMeshGeometry(const DecalBuildInfo& info, Vector<uint8>& buffer)
{
    const AABBox3 clipSpaceBox = AABBox3(Vector3(0.0f, 0.0f, 0.0f), 2.0f);

    char decalVertexData[MAX_CLIPPED_POLYGON_CAPACITY * sizeof(DecalVertex)];
    DecalVertex* points = reinterpret_cast<DecalVertex*>(decalVertexData);

    int32 geometryFormat = info.polygonGroup->GetFormat();
    uint32 triangleCount = static_cast<uint32>(info.polygonGroup->GetVertexCount() / 3);
    for (uint32 triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
    {
        int16 idx[3];
        info.polygonGroup->GetTriangleIndices(3 * triangleIndex, idx);
        for (int32 j = 0; j < 3; ++j)
        {
            info.polygonGroup->GetCoord(idx[j], points[j].originalPoint);

            if (geometryFormat & EVF_TEXCOORD0)
                info.polygonGroup->GetTexcoord(0, idx[j], points[j].texCoord0);
            if (geometryFormat & EVF_TEXCOORD1)
                info.polygonGroup->GetTexcoord(1, idx[j], points[j].texCoord1);
            if (geometryFormat & EVF_NORMAL)
                info.polygonGroup->GetNormal(idx[j], points[j].normal);
            if (geometryFormat & EVF_TANGENT)
                info.polygonGroup->GetTangent(idx[j], points[j].tangent);
            if (geometryFormat & EVF_BINORMAL)
                info.polygonGroup->GetBinormal(idx[j], points[j].binormal);

            info.polygonGroup->GetJointIndex(idx[j], points[j].jointIndex);
            DVASSERT(points[j].jointIndex < info.jointCount);

            Vector4 weightedVertexPosition = info.jointPositions[points[j].jointIndex];
            Vector4 weightedVertexQuaternion = info.jointQuaternions[points[j].jointIndex];
            Vector3 tmpVec = 2.0f * weightedVertexQuaternion.GetVector3().CrossProduct(points[j].originalPoint);
            points[j].actualPoint = weightedVertexPosition.GetVector3() + weightedVertexPosition.w *
            (points[j].originalPoint + weightedVertexQuaternion.w * tmpVec + weightedVertexQuaternion.GetVector3().CrossProduct(tmpVec));
        }

        if (Intersection::BoxTriangle(info.boundingBox, points[2].actualPoint, points[1].actualPoint, points[0].actualPoint))
        {
            Vector3 nrm = (points[1].actualPoint - points[0].actualPoint).CrossProduct(points[2].actualPoint - points[0].actualPoint);
            nrm.Normalize();
            if ((info.mapping != Mapping::PLANAR) || (nrm.DotProduct(info.projectionAxis) < -std::numeric_limits<float>::epsilon()))
            {
                AddVerticesToGeometry(info, points, buffer);
            }
        }
    }
}

bool GeoDecalManager::BuildDecal(const DecalBuildInfo& info, RenderBatch* dstBatch)
{
    if (info.polygonGroup == nullptr)
        return false;

    String fxName(info.material->GetEffectiveFXName().c_str());
    std::transform(fxName.begin(), fxName.end(), fxName.begin(), ::tolower);

    static const String InvalidMaterialsForDecal[] = { "shadow", "silhouette" };
    for (const String& m : InvalidMaterialsForDecal)
    {
        if (fxName.find(m) != String::npos)
        {
            // Logger::Warning("Material ignored for decal: %s", fxName.c_str());
            return false;
        }
    }

    int32 geometryFormat = info.polygonGroup->GetFormat();

    Vector<uint8_t> buffer;
    buffer.reserve(3 * sizeof(DecalVertex) * info.polygonGroup->GetIndexCount());
    if ((geometryFormat & EVF_JOINTINDEX) == EVF_JOINTINDEX)
    {
        GetSkinnedMeshGeometry(info, buffer);
    }
    else
    {
        GetStaticMeshGeometry(info, buffer);
    }

    if (buffer.empty())
        return false;

    uint32 decalVertexCount = static_cast<uint32>(buffer.size() / sizeof(DecalVertex));
    const DecalVertex* decalVertexPtr = reinterpret_cast<DecalVertex*>(buffer.data());

    ScopedPtr<PolygonGroup> newPolygonGroup(new PolygonGroup());
    newPolygonGroup->AllocateData(geometryFormat | EVF_DECAL_TEXCOORD, decalVertexCount, decalVertexCount);
    for (uint32 index = 0; index < decalVertexCount; ++index, ++decalVertexPtr)
    {
        const DecalVertex& c = (*decalVertexPtr);

        newPolygonGroup->SetCoord(index, c.originalPoint);
        newPolygonGroup->SetIndex(index, static_cast<int16>(index));
        newPolygonGroup->SetDecalTexcoord(index, c.decalCoord);

        if (geometryFormat & EVF_TEXCOORD0)
            newPolygonGroup->SetTexcoord(0, index, info.useCustomNormal ? Vector2(c.decalCoord.x, c.decalCoord.y) : c.texCoord0);

        if (geometryFormat & EVF_TEXCOORD1)
            newPolygonGroup->SetTexcoord(1, index, c.texCoord1);

        if (geometryFormat & EVF_NORMAL)
            newPolygonGroup->SetNormal(index, c.normal);

        if (geometryFormat & EVF_TANGENT)
            newPolygonGroup->SetTangent(index, c.tangent);

        if (geometryFormat & EVF_BINORMAL)
            newPolygonGroup->SetBinormal(index, c.binormal);

        if (geometryFormat & EVF_JOINTINDEX)
        {
            newPolygonGroup->SetJointCount(index, 1);
            newPolygonGroup->SetJointWeight(index, 0, 1.0f);
            newPolygonGroup->SetJointIndex(index, 0, c.jointIndex);
        }
    }
    newPolygonGroup->BuildBuffers();
    dstBatch->SetPolygonGroup(newPolygonGroup);
    dstBatch->UpdateAABBoxFromSource();

    /*
     * Process material
     */
    bool isBlinnFongMaterial = fxName.find("normalizedblinnphong") != String::npos;

    ScopedPtr<Texture> geoDecalTexture(Texture::CreateFromFile(info.albedo));

    ScopedPtr<NMaterial> material(new NMaterial());
    material->SetQualityGroup(info.material->GetQualityGroup());
    material->SetFXName(FastName(isBlinnFongMaterial ? "~res:/Materials/GeoDecal.Translucent.material" : "~res:/Materials/GeoDecal.material"));
    material->SetMaterialName(FastName("GeoDecal"));
    material->SetParent(info.material);
    material->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, geoDecalTexture);
    material->SetRuntime(true);

    if (fxName.find("lightmap") != String::npos)
    {
        material->AddFlag(FastName("MATERIAL_LIGHTMAP"), 1);
    }
    if (isBlinnFongMaterial)
    {
        bool perpixel = fxName.find("pervertex") == String::npos;
        material->AddFlag(FastName("NORMALIZED_BLINN_PHONG"), 1);
        material->AddFlag(FastName("PIXEL_LIT"), perpixel ? 1 : 0);
        material->AddFlag(FastName("VERTEX_LIT"), perpixel ? 0 : 1);

        if (info.useCustomNormal)
        {
            ScopedPtr<Texture> customNormal(Texture::CreateFromFile(info.normal));
            material->AddTexture(NMaterialTextureName::TEXTURE_NORMAL, customNormal);
        }
    }
    dstBatch->SetMaterial(material);
    return true;
}

int8_t GeoDecalManager::Classify(int8_t sign, Vector3::eAxis axis, const Vector3& c_v, const DecalVertex& p_v)
{
    int8_t ret = 0;
    float32 d = static_cast<float>(sign) * (p_v.actualPoint[axis] - c_v[axis]);

    if (d > PLANE_THICKNESS_EPSILON)
        ret = 1;

    if (d < -PLANE_THICKNESS_EPSILON)
        ret = -1;

    return 0;
}

void GeoDecalManager::Lerp(float t, const DecalVertex& v1, const DecalVertex& v2, DecalVertex& result)
{
    result.jointIndex = (t >= 0.5f) ? v2.jointIndex : v1.jointIndex;

#define LERP_IMPL(var) result.var = v1.var + (v2.var - v1.var) * t;
    LERP_IMPL(actualPoint);
    LERP_IMPL(originalPoint);
    LERP_IMPL(normal);
    LERP_IMPL(tangent);
    LERP_IMPL(binormal);
    LERP_IMPL(texCoord0);
    LERP_IMPL(texCoord1);
    LERP_IMPL(decalCoord);
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

    char new_p_vs_data[MAX_CLIPPED_POLYGON_CAPACITY * sizeof(DecalVertex)];
    DecalVertex* new_p_vs = reinterpret_cast<DecalVertex*>(new_p_vs_data);

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
                const float alpha = (p_v2.actualPoint[axis] - c_v[axis]) / (p_v2.actualPoint[axis] - p_v1.actualPoint[axis]);
                Lerp(alpha, p_v2, p_v1, new_p_vs[k]);
                ++k;
            }
            else if (d1 == 0 && (k == 0 || new_p_vs[k - 1].actualPoint != p_v1.actualPoint))
            {
                new_p_vs[k++] = p_v1;
            }
        }
        else if (d2 > 0)
        {
            polygonCompletelyOnPlane = false;
            if (d1 < 0)
            {
                const float alpha = (p_v2.actualPoint[axis] - c_v[axis]) / (p_v2.actualPoint[axis] - p_v1.actualPoint[axis]);
                Lerp(alpha, p_v2, p_v1, new_p_vs[k]);
                ++k;
            }
            else if (d1 == 0 && (k == 0 || new_p_vs[k - 1].actualPoint != p_v1.actualPoint))
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
