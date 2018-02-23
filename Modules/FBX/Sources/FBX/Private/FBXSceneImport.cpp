#include "FBXSceneImport.h"
#include "FBXMeshImport.h"

#include "Logger/Logger.h"
#include "FileSystem/FilePath.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/MeshComponent.h"
#include "Scene3D/Lod/LodComponent.h"

namespace DAVA
{
namespace FBXSceneImportDetails
{
void MoveChildrenEntities(Entity* fromEntity, Entity* toEntity);
}

namespace FBXImporterDetails
{
FbxScene* ImportFbxScene(FbxManager* fbxManager, const FilePath& fbxPath)
{
    FbxIOSettings* fbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
    FbxImporter* importer = FbxImporter::Create(fbxManager, "fbxImporter");

    bool initSuccess = importer->Initialize(fbxPath.GetAbsolutePathname().c_str());
    if (!initSuccess)
    {
        Logger::Error("FbxImporter Initialization error: %s", importer->GetStatus().GetErrorString());
        return nullptr;
    }

    FbxScene* fbxScene = FbxScene::Create(fbxManager, "importedScene");
    bool importSuccess = importer->Import(fbxScene);
    if (!importSuccess)
    {
        Logger::Error("FBX Import error: %s", importer->GetStatus().GetErrorString());
        return nullptr;
    }
    importer->Destroy();

    FbxAxisSystem::MayaZUp.ConvertScene(fbxScene); // UpVector = ZAxis, CoordSystem = RightHanded

    //FbxSystemUnit::ConvertScene() doesn't work properly in some cases, so we scale scene manually
    double conversionFactor = fbxScene->GetGlobalSettings().GetSystemUnit().GetConversionFactorTo(FbxSystemUnit::m);
    FbxVector4 rootNodeScaling = FbxVector4(fbxScene->GetRootNode()->LclScaling.Get()) * conversionFactor;
    fbxScene->GetRootNode()->LclScaling.Set(rootNodeScaling);

    return fbxScene;
}

void ProcessSceneHierarchyRecursive(FbxScene* fbxScene, Scene* scene)
{
    FbxNode* fbxRootNode = fbxScene->GetRootNode();

    int32 childCount = fbxRootNode->GetChildCount();
    for (int32 c = 0; c < childCount; ++c)
    {
        ScopedPtr<Entity> entity(new Entity());
        scene->AddEntity(entity);
        ProcessHierarchyRecursive(fbxRootNode->GetChild(c), entity);
    }
}

void ProcessHierarchyRecursive(FbxNode* fbxNode, Entity* entity)
{
    DVASSERT(entity != nullptr);

    entity->SetName(fbxNode->GetName());

    const FbxMesh* fbxMesh = fbxNode->GetMesh();
    if (fbxMesh != nullptr)
    {
        ImportMeshToEntity(fbxNode, entity);
    }
    else
    {
        Matrix4 transform = ToMatrix4(fbxNode->EvaluateLocalTransform());
        entity->SetLocalTransform(transform);
    }

    int32 childCount = fbxNode->GetChildCount();
    for (int32 c = 0; c < childCount; ++c)
    {
        ScopedPtr<Entity> childEntity(new Entity());
        entity->AddNode(childEntity);

        ProcessHierarchyRecursive(fbxNode->GetChild(c), childEntity);
    }
}

bool RemoveRedundantEntities(Entity* entity)
{
    Vector<Entity*> childrenToRemove;

    uint32 childrenCount = entity->GetChildrenCount();
    for (uint32 c = 0; c < childrenCount; ++c)
    {
        Entity* child = entity->GetChild(c);
        if (RemoveRedundantEntities(child))
            childrenToRemove.emplace_back(child);
    }

    for (Entity* e : childrenToRemove)
        entity->RemoveEntity(e);

    bool allowEntityCriteria = false;
    allowEntityCriteria |= (entity->GetChildrenCount() != 0u);
    allowEntityCriteria |= (entity->GetComponentCount<MeshComponent>() != 0u);

    return !allowEntityCriteria;
}

void ProcessMeshLODs(Entity* entity)
{
    Vector<Entity*> lod0Children;
    uint32 childrenCount = entity->GetChildrenCount();
    for (uint32 c = 0; c < childrenCount; ++c)
    {
        Entity* child = entity->GetChild(c);
        if (strstr(child->GetName().c_str(), "_lod0") != nullptr)
            lod0Children.emplace_back(child);
    }

    char strBuf[1024];
    for (Entity* lod0Child : lod0Children)
    {
        int32 maxLodIndex = -1;
        Vector<std::pair<Entity*, int32>> entitiesToCollapse;
        entitiesToCollapse.emplace_back(lod0Child, 0);

        const FastName& lod0ChildName = lod0Child->GetName();
        DVASSERT(strlen(lod0ChildName.c_str()) < sizeof(strBuf));

        strcpy(strBuf, lod0ChildName.c_str());
        char* lodMarker = strstr(strBuf, "_lod0");
        char* charToReplace = lodMarker + 4;

        for (int32 lodIndex = 1; lodIndex < LodComponent::MAX_LOD_LAYERS; ++lodIndex)
        {
            *charToReplace = char('0' + lodIndex);

            Entity* lodEntity = entity->FindByName(strBuf);
            if (lodEntity != nullptr)
                entitiesToCollapse.emplace_back(lodEntity, lodIndex);
        }

        *lodMarker = 0;
        lod0Child->SetName(strBuf);

        Vector<MeshLODDescriptor> meshDescriptor;
        for (auto& lodPair : entitiesToCollapse)
        {
            Entity* entityToCollapse = lodPair.first;
            int32 lodIndex = lodPair.second;

            MeshComponent* meshComponent = entityToCollapse->GetComponent<MeshComponent>();
            if (meshComponent != nullptr)
            {
                Vector<MeshLODDescriptor> desc = meshComponent->GetMeshDescriptor();
                for (MeshLODDescriptor& d : desc)
                    d.lodIndex = lodIndex;

                meshDescriptor.insert(meshDescriptor.end(), desc.begin(), desc.end());

                maxLodIndex = Max(maxLodIndex, lodIndex);
            }

            if (entityToCollapse != lod0Child)
            {
                FBXSceneImportDetails::MoveChildrenEntities(entityToCollapse, lod0Child);
                entityToCollapse->GetParent()->RemoveEntity(entityToCollapse);
            }
        }

        MeshComponent* meshComponent = lod0Child->GetComponent<MeshComponent>();
        DVASSERT(meshComponent != nullptr);
        meshComponent->SetMeshDescriptor(meshDescriptor);

        if (maxLodIndex != -1)
        {
            LodComponent* lodComponent = new LodComponent();
            lod0Child->AddComponent(lodComponent);
        }
    }

    childrenCount = entity->GetChildrenCount();
    for (uint32 c = 0; c < childrenCount; ++c)
        ProcessMeshLODs(entity->GetChild(c));
}
}; //ns FBXImporterDetails

namespace FBXSceneImportDetails
{
void MoveChildrenEntities(Entity* fromEntity, Entity* toEntity)
{
    DVASSERT(fromEntity != nullptr && toEntity != nullptr);

    if (fromEntity == toEntity)
        return;

    uint32 childrenCount = fromEntity->GetChildrenCount();
    Vector<Entity*> childrenToSwap;
    childrenToSwap.reserve(childrenCount);

    for (uint32 c = 0; c < childrenCount; ++c)
    {
        Entity* child = fromEntity->GetChild(c);
        child->Retain();
        childrenToSwap.emplace_back(child);
    }
    fromEntity->RemoveAllChildren();

    for (Entity* child : childrenToSwap)
        toEntity->AddEntity(child);
}
}; //ns FBXSceneImportDetails

}; //ns DAVA