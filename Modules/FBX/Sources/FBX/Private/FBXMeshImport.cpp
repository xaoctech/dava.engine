#include "FBXMeshImport.h"

#include "FBXMaterialImport.h"
#include "FBXSkeletonImport.h"

#include "Asset/Asset.h"
#include "Asset/AssetManager.h"

#include "Engine/Engine.h"

#include "FileSystem/FilePath.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Components/MeshComponent.h"
#include "Scene3D/Components/SkeletonComponent.h"

#include "Render/3D/MeshUtils.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/3D/Geometry.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/MeshLODDescriptor.h"
#include "Render/Highlevel/SkinnedMesh.h"
#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialNames.h"

#define FBX_IMPORT_AUTO_BUILD_TANGENT_SPACE 1

namespace DAVA
{
namespace FBXMeshImportDetails
{
//namespace declarations

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

Map<const FbxMesh*, MeshLODDescriptor> meshCache;
FilePath currentAssetsFolder;

FbxSurfaceMaterial* GetPolygonMaterial(FbxMesh* fbxMesh, int32 polygonIndex);

}; //ns FBXMeshImportDetails

namespace FBXImporterDetails
{
void SetGeometryAssetsFolder(const FilePath& filepath)
{
    DVASSERT(filepath.IsDirectoryPathname());
    FBXMeshImportDetails::currentAssetsFolder = filepath;
}

void ImportMeshToEntity(FbxNode* fbxNode, Entity* entity)
{
    using namespace FBXMeshImportDetails;

    DVASSERT(fbxNode);
    FbxMesh* fbxMesh = fbxNode->GetMesh();

    auto found = meshCache.find(fbxMesh);
    if (found == meshCache.end())
    {
        bool hasNormal = fbxMesh->GetElementNormalCount() > 0;
        bool hasTangent = fbxMesh->GetElementTangentCount() > 0;
        bool hasBinormal = fbxMesh->GetElementBinormalCount() > 0;
        bool hasSkinning = fbxMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;

        uint32 maxControlPointInfluence = 0;
        Vector<FbxControlPointInfluences> controlPointsInfluences;
        if (hasSkinning)
        {
            FbxSkin* fbxSkin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(0, FbxDeformer::eSkin));
            GetControlPointInfluences(fbxSkin, &controlPointsInfluences, &maxControlPointInfluence);
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

        using VerticesMap = std::pair<Map<FBXVertex, Vector<int32>>, int32>; //[[vertex, indices], polygonCount]
        using MaterialGeometryMap = Map<FbxSurfaceMaterial*, VerticesMap>;

        MaterialGeometryMap materialGeometry;
        int32 polygonCount = fbxMesh->GetPolygonCount();
        for (int32 p = 0; p < polygonCount; p++)
        {
            int32 polygonSize = fbxMesh->GetPolygonSize(p);
            DVASSERT(polygonSize == 3);

            FbxSurfaceMaterial* fbxMaterial = GetPolygonMaterial(fbxMesh, p);
            int32& materialPolygonIndex = materialGeometry[fbxMaterial].second;
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

                materialGeometry[fbxMaterial].first[vertex].push_back(materialPolygonIndex * 3 + v);
            }

            ++materialPolygonIndex;
        }

        FilePath geometryAssetPath = currentAssetsFolder + fbxNode->GetName() + ".geo";
        AssetManager* assetManager = GetEngineContext()->assetManager;
        Asset<Geometry> geometryAsset = assetManager->CreateAsset<Geometry>(Geometry::PathKey(geometryAssetPath));

        MeshLODDescriptor meshDescriptor;
        meshDescriptor.geometryAsset = geometryAsset;
        meshDescriptor.geometryPath = geometryAssetPath;

        Matrix4 meshTransform = ToMatrix4(fbxNode->EvaluateGlobalTransform());
        for (auto& gIt : materialGeometry)
        {
            FbxSurfaceMaterial* fbxMaterial = gIt.first;
            FilePath materialAssetPath = ImportMaterial(fbxMaterial, maxControlPointInfluence);

            const VerticesMap& vertices = gIt.second;
            int32 vxCount = int32(vertices.first.size());
            int32 indCount = int32(vertices.second * 3);

            PolygonGroup* polygonGroup = new PolygonGroup();
            polygonGroup->AllocateData(meshFormat, vxCount, indCount);

            int32 vertexIndex = 0;
            for (auto vIt = vertices.first.cbegin(); vIt != vertices.first.cend(); ++vIt)
            {
                const FBXVertex& fbxVertex = vIt->first;
                const Vector<int32>& indices = vIt->second;

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

                for (int32 index : indices)
                    polygonGroup->SetIndex(index, vertexIndex);

                ++vertexIndex;
            }

            polygonGroup->ApplyMatrix(meshTransform);

#if FBX_IMPORT_AUTO_BUILD_TANGENT_SPACE
            bool hasUV = uvCount != 0;
            bool completedTBN = hasNormal && hasTangent && hasBinormal;
            if (hasNormal && hasUV && !completedTBN)
            {
                MeshUtils::RebuildMeshTangentSpace(polygonGroup);
            }
#endif

            Vector<std::pair<PolygonGroup*, Mesh::JointTargets>> meshGeometry;
            if (hasSkinning)
            {
                meshGeometry = MeshUtils::SplitSkinnedMeshGeometry(polygonGroup, SkinnedMesh::MAX_TARGET_JOINTS);
                SafeRelease(polygonGroup);
            }
            else
            {
                meshGeometry.emplace_back(std::make_pair(polygonGroup, Mesh::JointTargets()));
            }

            for (auto& p : meshGeometry)
            {
                MeshBatchDescriptor meshBatch;

                PolygonGroup* pg = p.first;
                pg->RecalcAABBox();

                meshBatch.geometryIndex = geometryAsset->GetPolygonGroupCount();
                geometryAsset->AddPolygonGroup(pg);
                SafeRelease(pg);

                meshBatch.materialPath = materialAssetPath;
                meshBatch.materialAsset = GetEngineContext()->assetManager->FindAsset<Material>(materialAssetPath);

                meshBatch.jointTargets = p.second;

                meshDescriptor.batchDescriptors.emplace_back(std::move(meshBatch));
            }
        }

        assetManager->SaveAsset(geometryAsset);

        found = meshCache.emplace(fbxMesh, std::move(meshDescriptor)).first;
    }

    MeshComponent* meshComponent = new MeshComponent();
    meshComponent->SetMeshDescriptor({ found->second });
    entity->AddComponent(meshComponent);

    bool isSkinned = (found->first->GetDeformerCount(FbxDeformer::eSkin) > 0);
    if (isSkinned)
    {
        FbxSkin* fbxSkin = static_cast<FbxSkin*>(found->first->GetDeformer(0, FbxDeformer::eSkin));
        entity->AddComponent(GetBuiltSkeletonComponent(fbxSkin)->Clone(entity));
    }
}

void ClearMeshCache()
{
    FBXMeshImportDetails::meshCache.clear();
    FBXMeshImportDetails::currentAssetsFolder = FilePath();
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

FbxSurfaceMaterial* GetPolygonMaterial(FbxMesh* fbxMesh, int32 polygonIndex)
{
    FbxSurfaceMaterial* fbxMaterial = nullptr;
    for (int32 me = 0; me < fbxMesh->GetElementMaterialCount(); me++)
    {
        fbxMaterial = fbxMesh->GetNode()->GetMaterial(fbxMesh->GetElementMaterial(me)->GetIndexArray().GetAt(polygonIndex));
        if (fbxMaterial != nullptr)
            break;
    }

    return fbxMaterial;
}

}; //ns FBXMeshImportDetails

}; //ns DAVA
