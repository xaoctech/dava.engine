#include "FBXImporter.h"
#include "DAVAEngine.h"

#include <fbxsdk.h>

namespace DAVA
{
namespace FBXImporterDetails
{
struct FBXVertex
{
    FBXVertex()
    {
        Memset(data, 0, sizeof(data));
    }

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

    bool operator<(const FBXVertex& other) const
    {
        for (int32 d = 0; d < 14; ++d)
        {
            if (!FLOAT_EQUAL(data[d], other.data[d]))
                return data[d] < other.data[d];
        }

        return false;
    }
};

Matrix4 ToMatrix4(const FbxAMatrix& fbxMatrix)
{
    Matrix4 mx;

    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            mx._data[r][c] = float32(fbxMatrix.Get(r, c));

    return mx;
}

Mesh* ConvertMesh(FbxMesh* fbxMesh)
{
    static Map<FbxMesh*, PolygonGroup*> convertedPolygonGroups;

    ScopedPtr<PolygonGroup> polygonGroup;

    auto found = convertedPolygonGroups.find(fbxMesh);
    if (found == convertedPolygonGroups.end())
    {
        polygonGroup.reset(new PolygonGroup());

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

        Map<FBXVertex, Vector<int32>> geometry;
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

                geometry[vertex].push_back(p * 3 + v);
            }
        }

        int32 vxCount = int32(geometry.size());
        int32 indCount = polygonCount * 3;
        polygonGroup->AllocateData(meshFormat, vxCount, indCount);

        int32 vertexIndex = 0;
        for (auto it = geometry.cbegin(); it != geometry.cend(); ++it)
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

        //if (uvCount && hasNormal)
        //    MeshUtils::RebuildMeshTangentSpace(polygonGroup);
        //else
        //    polygonGroup->BuildBuffers();

        //////////////////////////////////////////////////////////////////////////

        /*
        int32 vxCount = fbxMesh->GetControlPointsCount();
        int32 indCount = fbxMesh->GetPolygonVertexCount();


#define EVALUATE_FBX_ELEMENT_INDEX(e, i) (((e)->GetReferenceMode() == FbxLayerElement::eIndexToDirect) ? (e)->GetIndexArray().GetAt((i)) : (i))

        polygonGroup->AllocateData(meshFormat, vxCount, indCount);
        for (int32 v = 0; v < vxCount; ++v)
        {
            FbxVector4 coords = fbxMesh->GetControlPointAt(v);
            polygonGroup->SetCoord(v, Vector3(float32(coords[0]), float32(coords[1]), float32(coords[2])));

            for (int32 t = 0; t < texCoordCount; ++t)
            {
                const FbxGeometryElementUV* uvElement = fbxMesh->GetElementUV(t);
                const FbxVector2& uv = uvElement->GetDirectArray().GetAt(EVALUATE_FBX_ELEMENT_INDEX(uvElement, v));
                polygonGroup->SetTexcoord(t, v, Vector2(float32(uv[0]), float32(uv[1])));
            }

            if (hasNormal)
            {
                const FbxGeometryElementNormal* normalElement = fbxMesh->GetElementNormal(0);
                const FbxVector4& normal = normalElement->GetDirectArray().GetAt(EVALUATE_FBX_ELEMENT_INDEX(normalElement, v));
                polygonGroup->SetNormal(v, Vector3(float32(normal[0]), float32(normal[1]), float32(normal[2])));
            }
        }

#undef EVALUATE_FBX_ELEMENT_INDEX

        for (int32 i = 0; i < indCount; ++i)
        {
            polygonGroup->SetIndex(i, int16(fbxMesh->GetPolygonVertices()[i]));
        }
        */
    }
    else
    {
        polygonGroup.reset((*found).second);
    }

    Mesh* mesh = new Mesh();
    ScopedPtr<NMaterial> material(new NMaterial());
    material->SetFXName(NMaterialName::TEXTURED_OPAQUE);
    material->SetMaterialName(FastName("bla-bla"));
    mesh->AddPolygonGroup(polygonGroup, material);

    return mesh;
}

void ProcessHierarchyRecursive(FbxNode* node, Entity* entity)
{
    entity->SetName(node->GetName());

    Matrix4 transform = ToMatrix4(node->EvaluateLocalTransform());
    entity->SetLocalTransform(transform);

    int attrCount = node->GetNodeAttributeCount();
    for (int a = 0; a < attrCount; ++a)
    {
        FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(a);
        if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            FbxMesh* mesh = static_cast<FbxMesh*>(attr);
            entity->AddComponent(new RenderComponent(ScopedPtr<Mesh>(FBXImporterDetails::ConvertMesh(mesh))));
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
}

void FBXImporter::ConvertToSC2(const FilePath& fbxPath)
{
    FbxManager* fbxManager = FbxManager::Create();

    FbxIOSettings* fbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
    fbxManager->SetIOSettings(fbxIOSettings);

    fbxIOSettings->SetBoolProp(IMP_FBX_MATERIAL, true);
    fbxIOSettings->SetBoolProp(IMP_FBX_TEXTURE, true);
    fbxIOSettings->SetBoolProp(IMP_FBX_LINK, true);
    fbxIOSettings->SetBoolProp(IMP_FBX_SHAPE, true);
    fbxIOSettings->SetBoolProp(IMP_FBX_GOBO, true);
    fbxIOSettings->SetBoolProp(IMP_FBX_ANIMATION, true);
    fbxIOSettings->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

    FbxImporter* importer = FbxImporter::Create(fbxManager, "fbxImporter");

    bool initSuccess = importer->Initialize(fbxPath.GetAbsolutePathname().c_str(), -1, fbxManager->GetIOSettings());
    if (!initSuccess)
    {
        Logger::Error("FBX Initialization error: %s", importer->GetStatus().GetErrorString());
        return;
    }

    FbxScene* fbxScene = FbxScene::Create(fbxManager, "importedScene");
    importer->Import(fbxScene);
    importer->Destroy();

    FbxAxisSystem(FbxAxisSystem::EPreDefinedAxisSystem::eMax).ConvertScene(fbxScene); // UpVector = ZAxis, CoordSystem = RightHanded
    //FbxSystemUnit(100.0).ConvertScene(fbxScene); // cm -> m
    FbxGeometryConverter* fbxGeometryConverter = new FbxGeometryConverter(fbxManager);
    fbxGeometryConverter->Triangulate(fbxScene, true);
    SafeDelete(fbxGeometryConverter);

    Scene* scene = new Scene();
    FBXImporterDetails::ProcessHierarchyRecursive(fbxScene->GetRootNode(), scene);

    scene->SaveScene(FilePath::CreateWithNewExtension(fbxPath, ".sc2"));

    fbxManager->Destroy();
}
};
