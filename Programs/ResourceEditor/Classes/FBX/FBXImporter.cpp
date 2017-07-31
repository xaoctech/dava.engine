#include "FBXImporter.h"
#include "DAVAEngine.h"

#define FBXSDK_SHARED //requested only for dynamic linking

#include <fbxsdk.h>

namespace DAVA
{
namespace FBXImporterDetails
{
struct FBXVertex
{
    union
    {
        float32 data[14];
        struct
        {
            Vector3 position;
            Vector2 texCoord[4];
            Vector3 normal;
        };
    };

    FBXVertex();

    bool operator<(const FBXVertex& other) const;
};

//////////////////////////////////////////////////////////////////////////

void ProcessHierarchyRecursive(FbxNode* node, Entity* entity);
void ClearCache();

RenderObject* CreateRenderObject(FbxMesh* fbxMesh);
NMaterial* RetrieveMaterial(FbxSurfaceMaterial* fbxMaterial);
SkeletonComponent* RetrieveSkeleton(FbxSkeleton* skeleton);

Matrix4 ToMatrix4(const FbxAMatrix& fbxMatrix);
const char* GetFBXTexturePath(const FbxProperty& textureProperty);

//////////////////////////////////////////////////////////////////////////

using GeometrySet = Vector<std::pair<PolygonGroup*, NMaterial*>>;

Map<FbxMesh*, GeometrySet> geometryCache; //in geometry cache material isn't retained
Map<FbxSurfaceMaterial*, NMaterial*> materialCache;
Map<FbxSkeleton*, SkeletonComponent*> skeletonCache;

static uint32 materialInstanceIndex = 0;
}

//////////////////////////////////////////////////////////////////////////

bool FBXImporter::ConvertToSC2(const FilePath& fbxPath, const FilePath& sc2Path)
{
    FbxManager* fbxManager = FbxManager::Create();

    FbxIOSettings* fbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
    FbxImporter* importer = FbxImporter::Create(fbxManager, "fbxImporter");

    bool initSuccess = importer->Initialize(fbxPath.GetAbsolutePathname().c_str());
    if (!initSuccess)
    {
        Logger::Error("FBX Initialization error: %s", importer->GetStatus().GetErrorString());
        return false;
    }

    FbxScene* fbxScene = FbxScene::Create(fbxManager, "importedScene");
    bool importSuccess = importer->Import(fbxScene);
    if (!importSuccess)
    {
        Logger::Error("FBX Import error: %s", importer->GetStatus().GetErrorString());
        return false;
    }
    importer->Destroy();

    FbxAxisSystem::MayaZUp.ConvertScene(fbxScene); // UpVector = ZAxis, CoordSystem = RightHanded
    FbxGeometryConverter fbxGeometryConverter(fbxManager);
    fbxGeometryConverter.Triangulate(fbxScene, true); //Triangulate whole scene

    ScopedPtr<Scene> scene(new Scene());
    FBXImporterDetails::ProcessHierarchyRecursive(fbxScene->GetRootNode(), scene);
    FBXImporterDetails::ClearCache();
    fbxScene->Destroy();

    scene->SaveScene(sc2Path);

    fbxManager->Destroy();

    return true;
}

//////////////////////////////////////////////////////////////////////////
//Details implementation
namespace FBXImporterDetails
{
void ProcessHierarchyRecursive(FbxNode* node, Entity* entity)
{
    entity->SetName(node->GetName());

    Matrix4 transform = ToMatrix4(node->EvaluateLocalTransform());
    entity->SetLocalTransform(transform);

    int attrCount = node->GetNodeAttributeCount();
    for (int32 a = 0; a < attrCount; ++a)
    {
        FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(a);
        if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            FbxMesh* fbxMesh = static_cast<FbxMesh*>(attr);
            entity->AddComponent(new RenderComponent(ScopedPtr<RenderObject>(CreateRenderObject(fbxMesh))));
        }
        else if (attr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
        {
            FbxSkeleton* fbxSkeleton = static_cast<FbxSkeleton*>(attr);
            entity->AddComponent(RetrieveSkeleton(fbxSkeleton)->Clone(nullptr));
        }
    }

    int childCount = node->GetChildCount();
    for (int c = 0; c < childCount; ++c)
    {
        ScopedPtr<Entity> childEntity(new Entity());
        entity->AddNode(childEntity);

        ProcessHierarchyRecursive(node->GetChild(c), childEntity);
    }
}

void ClearCache()
{
    for (auto& it : geometryCache)
    {
        for (auto& p : it.second)
            SafeRelease(p.first);
        //in geometry cache material isn't retained
    }
    geometryCache.clear();

    for (auto& it : materialCache)
    {
        SafeRelease(it.second);
    }
    materialCache.clear();

    for (auto& it : skeletonCache)
    {
        SafeDelete(it.second);
    }
    skeletonCache.clear();

    materialInstanceIndex = 0;
}

//////////////////////////////////////////////////////////////////////////

RenderObject* CreateRenderObject(FbxMesh* fbxMesh)
{
    auto found = geometryCache.find(fbxMesh);
    if (found == geometryCache.end())
    {
        FbxNode* fbxNode = fbxMesh->GetNode();
        DVASSERT(fbxNode);

        FbxStringList uvNames;
        fbxMesh->GetUVSetNames(uvNames);
        int32 uvCount = Min(uvNames.GetCount(), 4);

        bool hasNormal = fbxMesh->GetElementNormalCount() > 0;

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
            meshFormat |= EVF_NORMAL | EVF_TANGENT | EVF_BINORMAL;

        FbxVector4 tmpNormal;
        FbxVector2 tmpUV;
        bool tmpUnmapped = false;

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
                const FbxVector4& coords = fbxMesh->GetControlPointAt(vIndex);
                vertex.position = Vector3(coords[0], coords[1], coords[2]);

                fbxMesh->GetPolygonVertexNormal(p, v, tmpNormal);
                vertex.normal = Vector3(tmpNormal[0], tmpNormal[1], tmpNormal[2]);

                for (int32 t = 0; t < uvCount; ++t)
                {
                    fbxMesh->GetPolygonVertexUV(p, v, uvNames[t], tmpUV, tmpUnmapped);
                    vertex.texCoord[t] = Vector2(tmpUV[0], -tmpUV[1]);
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
                polygonGroup->SetCoord(vertexIndex, it->first.position);

                for (int32 t = 0; t < uvCount; ++t)
                    polygonGroup->SetTexcoord(t, vertexIndex, it->first.texCoord[t]);

                if (hasNormal)
                    polygonGroup->SetNormal(vertexIndex, it->first.normal);

                for (int32 index : it->second)
                    polygonGroup->SetIndex(index, uint16(vertexIndex));

                ++vertexIndex;
            }

            if (hasNormal)
                MeshUtils::RebuildMeshTangentSpace(polygonGroup);

            geometrySet.emplace_back(polygonGroup, RetrieveMaterial(fbxMaterial));
        }

        found = geometryCache.emplace(fbxMesh, std::move(geometrySet)).first;
    }

    Mesh* renderObject = new Mesh();

    const GeometrySet& geometrySet = found->second;
    for (auto& geometry : geometrySet)
    {
        PolygonGroup* polygonGroup = geometry.first;
        NMaterial* material = geometry.second;

        ScopedPtr<NMaterial> materialInstance(new NMaterial());
        materialInstance->SetParent(material);
        materialInstance->SetMaterialName(FastName(Format("Instance%d", materialInstanceIndex++).c_str()));

        renderObject->AddPolygonGroup(polygonGroup, materialInstance);
    }

    return renderObject;
}

NMaterial* RetrieveMaterial(FbxSurfaceMaterial* fbxMaterial)
{
    auto found = materialCache.find(fbxMaterial);
    if (found == materialCache.end())
    {
        NMaterial* material = new NMaterial();
        material->SetFXName(NMaterialName::TEXTURED_OPAQUE);
        material->SetMaterialName(FastName(fbxMaterial->GetName()));

        Vector<std::pair<const char*, FastName>> texturesToImport = {
            { FbxSurfaceMaterial::sDiffuse, NMaterialTextureName::TEXTURE_ALBEDO },
            { FbxSurfaceMaterial::sNormalMap, NMaterialTextureName::TEXTURE_NORMAL }
        };

        for (auto& tex : texturesToImport)
        {
            const char* texturePath = GetFBXTexturePath(fbxMaterial->FindProperty(tex.first));
            if (texturePath)
                material->AddTexture(tex.second, Texture::CreateFromFile(FilePath(texturePath)));
        }

        found = materialCache.emplace(fbxMaterial, material).first;
    }

    return found->second;
}

SkeletonComponent* RetrieveSkeleton(FbxSkeleton* fbxSkeleton)
{
    auto found = skeletonCache.find(fbxSkeleton);
    if (found == skeletonCache.end())
    {
        SkeletonComponent* skeleton = new SkeletonComponent();

        //TODO

        found = skeletonCache.emplace(fbxSkeleton, skeleton).first;
    }

    return found->second;
}

//////////////////////////////////////////////////////////////////////////

FBXVertex::FBXVertex()
{
    Memset(data, 0, sizeof(data));
}

inline bool FBXVertex::operator<(const FBXVertex& other) const
{
    for (int32 d = 0; d < 14; ++d)
    {
        if (!FLOAT_EQUAL(data[d], other.data[d]))
            return data[d] < other.data[d];
    }

    return false;
}

Matrix4 ToMatrix4(const FbxAMatrix& fbxMatrix)
{
    Matrix4 mx;

    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            mx._data[r][c] = float32(fbxMatrix.Get(r, c));

    return mx;
}

const char* GetFBXTexturePath(const FbxProperty& textureProperty)
{
    FbxTexture* fbxTexture = nullptr;
    if (textureProperty.GetSrcObjectCount<FbxLayeredTexture>() > 0)
    {
        FbxLayeredTexture* layeredTexture = textureProperty.GetSrcObject<FbxLayeredTexture>(0);
        if (layeredTexture->GetSrcObjectCount<FbxTexture>() > 0)
            fbxTexture = layeredTexture->GetSrcObject<FbxTexture>(0);
    }
    else
    {
        if (textureProperty.GetSrcObjectCount<FbxTexture>() > 0)
            textureProperty.GetSrcObject<FbxTexture>(0);
    }

    FbxFileTexture* fbxFileTexture = FbxCast<FbxFileTexture>(fbxTexture);
    if (fbxFileTexture)
        return fbxFileTexture->GetFileName();

    return nullptr;
}

}; //ns Details

}; //ns DAVA
