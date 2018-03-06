#include "Scene3D/AssetLoaders/MaterialAssetLoader.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/SceneFile/VersionInfo.h"

#include "Base/Any.h"
#include "Debug/DVAssert.h"
#include "Engine/Engine.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/YamlParser.h"
#include "Render/Material/Material.h"
#include "Render/Material/NMaterial.h"
#include "Asset/AssetManager.h"
#include "Engine/EngineContext.h"

#define MATERIAL_ASSET_YAML_SERIALIZATION 1

namespace DAVA
{
namespace MaterialAssetLoaderDetail
{
size_t PathKeyHash(const Any& v)
{
    const MaterialAssetLoader::PathKey& key = v.Get<MaterialAssetLoader::PathKey>();
    std::hash<String> hashFn;
    return hashFn(key.path.GetAbsolutePathname());
}
} // namespace MaterialAssetLoaderDetail

MaterialAssetLoader::MaterialAssetLoader()
{
    AnyHash<MaterialAssetLoader::PathKey>::Register(&MaterialAssetLoaderDetail::PathKeyHash);
}

AssetFileInfo MaterialAssetLoader::GetAssetFileInfo(const Any& assetKey) const
{
    DVASSERT(assetKey.CanGet<PathKey>());
    const PathKey& key = assetKey.Get<PathKey>();
    AssetFileInfo info;
    info.fileName = key.path.GetAbsolutePathname();

    return info;
}

AssetBase* MaterialAssetLoader::CreateAsset(const Any& assetKey) const
{
    return new Material(assetKey);
}

void MaterialAssetLoader::DeleteAsset(AssetBase* asset) const
{
    DVASSERT(dynamic_cast<Material*>(asset) != nullptr);
    delete asset;
}

void MaterialAssetLoader::LoadAsset(Asset<AssetBase> asset, File* file, String& errorMessage) const
{
    Asset<Material> materialAsset = std::dynamic_pointer_cast<Material>(asset);
    SerializationContext serializationContext;
    serializationContext.SetScenePath(file->GetFilename().GetDirectory());
    serializationContext.SetVersion(STREAMING_SCENE_VERSION);

    ScopedPtr<KeyedArchive> archive(new KeyedArchive());

    ScopedPtr<YamlParser> yamlParser(YamlParser::Create(file->GetFilename()));
    if (yamlParser)
    {
        archive->LoadFromYamlNode(yamlParser->GetRootNode());
    }
    else
    {
        archive->Load(file);
    }

    materialAsset->material = new NMaterial();
    materialAsset->material->Load(archive, &serializationContext);

    if (archive->IsKeyExists("parentPath"))
    {
        materialAsset->parentPath = serializationContext.GetScenePath() + archive->GetString("parentPath");
        materialAsset->parentAsset = GetEngineContext()->assetManager->GetAsset<Material>(PathKey(materialAsset->parentPath), AssetManager::SYNC);

        materialAsset->material->SetParent(materialAsset->parentAsset->GetMaterial());
    }
}

bool MaterialAssetLoader::SaveAsset(Asset<AssetBase> asset, File* file, eSaveMode requestedMode) const
{
    Asset<Material> materialAsset = std::dynamic_pointer_cast<Material>(asset);
    SerializationContext serializationContext;
    serializationContext.SetScenePath(file->GetFilename().GetDirectory());
    serializationContext.SetVersion(STREAMING_SCENE_VERSION);

    if (materialAsset->material != nullptr)
    {
        ScopedPtr<KeyedArchive> archive(new KeyedArchive());

        materialAsset->material->Save(archive, &serializationContext);
        archive->DeleteKey(NMaterialSerializationKey::ParentMaterialKey);

        if (!materialAsset->parentPath.IsEmpty())
            archive->SetString("parentPath", materialAsset->parentPath.GetRelativePathname(serializationContext.GetScenePath()));

#if MATERIAL_ASSET_YAML_SERIALIZATION
        archive->SaveToYamlFile(file->GetFilename());
#else
        archive->Save(file);
#endif
    }

    return true;
}

bool MaterialAssetLoader::SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const
{
    return false;
}

Vector<const Type*> MaterialAssetLoader::GetAssetKeyTypes() const
{
    return Vector<const Type*>{ Type::Instance<PathKey>() };
}

Vector<const Type*> MaterialAssetLoader::GetAssetTypes() const
{
    return Vector<const Type*>{ Type::Instance<Material>() };
}

template <>
bool AnyCompare<MaterialAssetLoader::PathKey>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<MaterialAssetLoader::PathKey>().path == v2.Get<MaterialAssetLoader::PathKey>().path;
}
} // namespace DAVA
