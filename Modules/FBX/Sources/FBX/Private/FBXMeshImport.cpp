#include "FBXMeshImport.h"

#include "FBXMaterialImport.h"
#include "FBXSkeletonImport.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/RenderComponent.h"

#include "Render/3D/MeshUtils.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/SkinnedMesh.h"
#include "Render/Material/NMaterial.h"

namespace DAVA
{
namespace FBXMeshImportDetails
{
//namespace declarations

using GeometrySet = Vector<std::pair<PolygonGroup*, NMaterial*>>;

struct ProcessedMesh
{
    GeometrySet geometry;
    SkeletonComponent* skeleton = nullptr;
    uint32 maxVertexInfluenceCount = 0;
};

struct FBXVertex
{
    struct JointWeightComparator
    {
        bool operator()(const FBXImporterDetails::VertexInfluence& l, const FBXImporterDetails::VertexInfluence& r) const
        {
            return l.second > r.second; //for sorting in descending order
        }
    };

    union
    {
        float32 data[20];
        struct
        {
            Vector3 position;
            Vector2 texCoord[4];
            Vector3 normal;
            Vector3 tangent;
            Vector3 binormal;
        };
    };

    FBXVertex();
    FBXVertex(const FBXVertex& other);

    bool operator<(const FBXVertex& other) const;

    Set<FBXImporterDetails::VertexInfluence, JointWeightComparator> joints;
};

//namespace members

static uint32 materialInstanceIndex = 0;
Map<const FbxNode*, ProcessedMesh> meshCache; //in ProcessedMesh::GeometrySet materials isn't retained. It's owned by materialCache

}; //ns FBXMeshImportDetails

namespace FBXImporterDetails
{
void ImportMeshToEntity(FbxNode* fbxNode, Entity* entity)
{
    using namespace FBXMeshImportDetails;

    auto found = meshCache.find(fbxNode);
    if (found == meshCache.end())
    {
        const FbxMesh* fbxMesh = fbxNode->GetMesh();
        DVASSERT(fbxNode);

        FbxAMatrix meshTransform = fbxNode->EvaluateGlobalTransform();

        bool hasNormal = fbxMesh->GetElementNormalCount() > 0;
        bool hasTangent = fbxMesh->GetElementTangentCount() > 0;
        bool hasBinormal = fbxMesh->GetElementBinormalCount() > 0;
        bool hasSkinning = fbxMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;

        uint32 maxControlPointInfluence = 0;
        Vector<FbxControlPointInfluences> controlPointsInfluences;
        SkeletonComponent* skeleton = nullptr;
        if (hasSkinning)
        {
            FbxSkin* skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(0, FbxDeformer::eSkin));
            skeleton = ImportSkeleton(skin, meshTransform, &controlPointsInfluences, &maxControlPointInfluence);
        }
        maxControlPointInfluence = Min(maxControlPointInfluence, PolygonGroup::MAX_VERTEX_JOINTS_COUNT);

        FbxStringList uvNames;
        fbxMesh->GetUVSetNames(uvNames);
        int32 uvCount = Min(uvNames.GetCount(), 4);

        int32 meshFormat = EVF_VERTEX;
        if (uvCount > 0)
            meshFormat |= EVF_TEXCOORD0;
        if (uvCount > 1)
            meshFormat |= EVF_TEXCOORD1;
        if (uvCount > 2)
            meshFormat |= EVF_TEXCOORD2;
        if (uvCount > 3)
            meshFormat |= EVF_TEXCOORD3;
        if (hasNormal)
            meshFormat |= EVF_NORMAL;
        if (hasTangent)
            meshFormat |= EVF_TANGENT;
        if (hasBinormal)
            meshFormat |= EVF_BINORMAL;
        if (maxControlPointInfluence == 1)
            meshFormat |= EVF_HARD_JOINTINDEX;
        if (maxControlPointInfluence > 1)
            meshFormat |= EVF_JOINTINDEX | EVF_JOINTWEIGHT;

        using VerticesMap = Map<FBXVertex, Vector<int32>>; //[vertex, indices]
        using MaterialGeometryMap = Map<FbxSurfaceMaterial*, VerticesMap>;

        MaterialGeometryMap materialGeometry;
        int32 polygonCount = fbxMesh->GetPolygonCount();
        for (int32 p = 0; p < polygonCount; p++)
        {
            int32 polygonSize = fbxMesh->GetPolygonSize(p);
            DVASSERT(polygonSize == 3);

            for (int32 v = 0; v < polygonSize; ++v)
            {
                FBXVertex vertex;

                int32 vIndex = fbxMesh->GetPolygonVertex(p, v);
                vertex.position = ToVector3(fbxMesh->GetControlPointAt(vIndex));

                if (hasNormal)
                    vertex.normal = ToVector3(GetFbxMeshLayerElementValue<FbxVector4>(fbxMesh->GetElementNormal(), vIndex, p, v));

                if (hasTangent)
                    vertex.tangent = ToVector3(GetFbxMeshLayerElementValue<FbxVector4>(fbxMesh->GetElementTangent(), vIndex, p, v));

                if (hasBinormal)
                    vertex.binormal = ToVector3(GetFbxMeshLayerElementValue<FbxVector4>(fbxMesh->GetElementBinormal(), vIndex, p, v));

                for (int32 t = 0; t < uvCount; ++t)
                {
                    vertex.texCoord[t] = ToVector2(GetFbxMeshLayerElementValue<FbxVector2>(fbxMesh->GetElementUV(uvNames[t]), vIndex, p, v));
                    vertex.texCoord[t].y = -vertex.texCoord[t].y;
                }

                if (hasSkinning)
                {
                    const FbxControlPointInfluences& vertexInfluences = controlPointsInfluences[vIndex];

                    float32 weightsSum = 0.f;
                    for (const VertexInfluence& vInf : vertexInfluences)
                        weightsSum += vInf.second;

                    for (const VertexInfluence& vInf : vertexInfluences)
                        vertex.joints.insert(VertexInfluence(vInf.first, vInf.second / weightsSum));
                }

                FbxSurfaceMaterial* fbxMaterial = nullptr;
                for (int32 me = 0; me < fbxMesh->GetElementMaterialCount(); me++)
                {
                    fbxMaterial = fbxNode->GetMaterial(fbxMesh->GetElementMaterial(me)->GetIndexArray().GetAt(p));
                    if (fbxMaterial != nullptr)
                        break;
                }

                materialGeometry[fbxMaterial][vertex].push_back(p * 3 + v);
            }
        }

        GeometrySet geometrySet;
        for (auto& it : materialGeometry)
        {
            FbxSurfaceMaterial* fbxMaterial = it.first;
            const VerticesMap& vertices = it.second;

            int32 vxCount = int32(vertices.size());
            int32 indCount = polygonCount * 3;

            PolygonGroup* polygonGroup = new PolygonGroup();
            polygonGroup->AllocateData(meshFormat, vxCount, indCount);

            int32 vertexIndex = 0;
            for (auto it = vertices.cbegin(); it != vertices.cend(); ++it)
            {
                const FBXVertex& fbxVertex = it->first;

                polygonGroup->SetCoord(vertexIndex, fbxVertex.position);

                for (int32 t = 0; t < uvCount; ++t)
                    polygonGroup->SetTexcoord(t, vertexIndex, fbxVertex.texCoord[t]);

                if (hasNormal)
                    polygonGroup->SetNormal(vertexIndex, fbxVertex.normal);

                if (hasTangent)
                    polygonGroup->SetTangent(vertexIndex, fbxVertex.tangent);

                if (hasBinormal)
                    polygonGroup->SetBinormal(vertexIndex, fbxVertex.binormal);

                if (hasSkinning)
                {
                    if (maxControlPointInfluence == 1) //hard-skinning
                    {
                        polygonGroup->SetHardJointIndex(vertexIndex, int32(fbxVertex.joints.cbegin()->first));
                    }
                    else
                    {
                        auto vInf = fbxVertex.joints.cbegin();
                        for (uint32 j = 0; j < PolygonGroup::MAX_VERTEX_JOINTS_COUNT; ++j)
                        {
                            if (vInf != fbxVertex.joints.end())
                            {
                                polygonGroup->SetJointIndex(vertexIndex, j, int32(vInf->first));
                                polygonGroup->SetJointWeight(vertexIndex, j, vInf->second);

                                ++vInf;
                            }
                            else
                            {
                                polygonGroup->SetJointIndex(vertexIndex, j, 0);
                                polygonGroup->SetJointWeight(vertexIndex, j, 0.f);
                            }
                        }
                    }
                }

                for (int32 index : it->second)
                    polygonGroup->SetIndex(index, uint16(vertexIndex));

                ++vertexIndex;
            }

            polygonGroup->ApplyMatrix(ToMatrix4(meshTransform));

            geometrySet.emplace_back(polygonGroup, ImportMaterial(fbxMaterial, maxControlPointInfluence));
        }

        found = meshCache.emplace(fbxNode, ProcessedMesh()).first;
        found->second.geometry = std::move(geometrySet);
        found->second.skeleton = skeleton;
        found->second.maxVertexInfluenceCount = maxControlPointInfluence;
    }

    bool isSkinned = (found->second.skeleton != nullptr);
    if (isSkinned)
    {
        ScopedPtr<SkinnedMesh> mesh(new SkinnedMesh());

        uint32 maxVertexInfluenceCount = found->second.maxVertexInfluenceCount;
        const GeometrySet& geometrySet = found->second.geometry;
        for (auto& geometry : geometrySet)
        {
            PolygonGroup* polygonGroup = geometry.first;
            NMaterial* material = geometry.second;

            ScopedPtr<NMaterial> materialInstance(new NMaterial());
            materialInstance->SetParent(material);
            materialInstance->SetMaterialName(FastName(Format("Instance%d", materialInstanceIndex++).c_str()));

            auto splitedPolygons = MeshUtils::SplitSkinnedMeshGeometry(polygonGroup, SkinnedMesh::MAX_TARGET_JOINTS);
            for (auto& p : splitedPolygons)
            {
                PolygonGroup* pg = p.first;
                pg->RecalcAABBox();

                ScopedPtr<RenderBatch> renderBatch(new RenderBatch());
                renderBatch->SetPolygonGroup(pg);
                renderBatch->SetMaterial(materialInstance);

                mesh->AddRenderBatch(renderBatch);
                mesh->SetJointTargets(renderBatch, p.second);

                SafeRelease(pg);
            }
        }

        entity->AddComponent(new RenderComponent(mesh));
        entity->AddComponent(found->second.skeleton->Clone(entity));
    }
    else
    {
        ScopedPtr<Mesh> mesh(new Mesh());

        const GeometrySet& geometrySet = found->second.geometry;
        for (auto& geometry : geometrySet)
        {
            PolygonGroup* polygonGroup = geometry.first;
            NMaterial* material = geometry.second;

            ScopedPtr<NMaterial> materialInstance(new NMaterial());
            materialInstance->SetParent(material);
            materialInstance->SetMaterialName(FastName(Format("Instance%d", materialInstanceIndex++).c_str()));

            mesh->AddPolygonGroup(polygonGroup, materialInstance);
        }

        entity->AddComponent(new RenderComponent(mesh));
    }
}

void ClearMeshCache()
{
    for (auto& it : FBXMeshImportDetails::meshCache)
    {
        SafeDelete(it.second.skeleton);

        for (auto& p : it.second.geometry)
            SafeRelease(p.first);
        //in geometry cache material isn't retained
    }
    FBXMeshImportDetails::meshCache.clear();

    FBXMeshImportDetails::materialInstanceIndex = 0;
}

}; //ns FBXImporterDetails

//////////////////////////////////////////////////////////////////////////
//FBXMeshImportDetails namespace definitions

namespace FBXMeshImportDetails
{
FBXVertex::FBXVertex()
{
    Memset(data, 0, sizeof(data));
}

FBXVertex::FBXVertex(const FBXVertex& other)
    : joints(other.joints)
{
    Memcpy(data, other.data, sizeof(data));
}

bool FBXVertex::operator<(const FBXVertex& other) const
{
    for (int32 d = 0; d < 14; ++d)
    {
        if (!FLOAT_EQUAL(data[d], other.data[d]))
            return data[d] < other.data[d];
    }

    if (joints.size() != other.joints.size() || joints.size() == 0)
        return joints.size() < other.joints.size();

    auto i = joints.cbegin();
    auto j = other.joints.cbegin();
    while (i != joints.cend())
    {
        if (i->first != j->first)
            return i->first < j->first;

        if (!FLOAT_EQUAL(i->second, j->second))
            return i->second < j->second;

        ++i;
        ++j;
    }

    return false;
}

}; //ns FBXMeshImportDetails

}; //ns DAVA
