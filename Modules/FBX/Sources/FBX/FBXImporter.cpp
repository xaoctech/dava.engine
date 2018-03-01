#include "FBXImporter.h"

#include "Private/FBXAnimationImport.h"
#include "Private/FBXMaterialImport.h"
#include "Private/FBXMeshImport.h"
#include "Private/FBXSceneImport.h"
#include "Private/FBXSkeletonImport.h"
#include "Private/FBXUtils.h"

#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/FileSystem.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Prefab.h"
#include "Scene3D/SceneUtils.h"
#include "Scene3D/AssetLoaders/PrefabAssetLoader.h"

namespace DAVA
{
Scene* FBXImporter::ConstructSceneFromFBX(const FilePath& fbxPath)
{
    FbxManager* fbxManager = FbxManager::Create();

    Scene* scene = nullptr;
    FbxScene* fbxScene = FBXImporterDetails::ImportFbxScene(fbxManager, fbxPath);
    if (fbxScene != nullptr)
    {
        scene = new Scene();

        FbxGeometryConverter fbxGeometryConverter(fbxManager);
        fbxGeometryConverter.Triangulate(fbxScene, true); //Triangulate whole scene

        FilePath assetsPath = fbxPath.GetDirectory() + "assets/";
        FileSystem::Instance()->CreateDirectory(assetsPath, true);

        FBXImporterDetails::SetMaterialAssetsFolder(assetsPath);
        FBXImporterDetails::SetGeometryAssetsFolder(assetsPath);

        FBXImporterDetails::ProcessSceneSkeletons(fbxScene);
        FBXImporterDetails::ProcessSceneHierarchyRecursive(fbxScene, scene);

        FBXImporterDetails::RemoveRedundantEntities(scene);
        FBXImporterDetails::ProcessMeshLODs(scene);

        fbxScene->Destroy();
    }
    fbxManager->Destroy();

    FBXImporterDetails::ClearMaterialCache();
    FBXImporterDetails::ClearMeshCache();
    FBXImporterDetails::ClearSkeletonCache();
    FBXImporterDetails::ClearNodeUIDCache();

    return scene;
}

bool FBXImporter::ConvertToPrefab(const FilePath& fbxPath, const FilePath& prefabPath)
{
    ScopedPtr<Scene> scene(ConstructSceneFromFBX(fbxPath));
    if (scene)
    {
        PrefabAssetLoader::PathKey assetKey(prefabPath);
        return GetEngineContext()->assetManager->SaveAssetFromData(scene, assetKey);
    }

    return false;
}

bool FBXImporter::ConvertAnimations(const FilePath& fbxPath)
{
    FbxManager* fbxManager = FbxManager::Create();

    FbxScene* fbxScene = FBXImporterDetails::ImportFbxScene(fbxManager, fbxPath);
    if (fbxScene != nullptr)
    {
        Vector<FBXImporterDetails::FBXAnimationStackData> animations = FBXImporterDetails::ImportAnimations(fbxScene);

        for (FBXImporterDetails::FBXAnimationStackData& animation : animations)
        {
            FilePath animationPath = (animations.size() == 1) ? FilePath::CreateWithNewExtension(fbxPath, ".anim") : FilePath::CreateWithNewExtension(fbxPath, "_" + animation.name + ".anim");
            FBXImporterDetails::SaveAnimation(animation, animationPath);
        }

        fbxScene->Destroy();
    }
    fbxManager->Destroy();

    FBXImporterDetails::ClearNodeUIDCache();

    return (fbxScene != nullptr);
}

}; //ns DAVA
