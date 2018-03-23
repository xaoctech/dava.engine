#include "Scene3D/AssetLoaders/MaterialAssetLoader.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/SceneFile/VersionInfo.h"

#include "Asset/AssetManager.h"
#include "Base/Any.h"
#include "Debug/DVAssert.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/YamlParser.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Material/Material.h"
#include "Render/Material/NMaterial.h"
#include "Reflection/ReflectionRegistrator.h"

#define MATERIAL_ASSET_YAML_SERIALIZATION 1

namespace DAVA
{
namespace MaterialAssetLoaderDetail
{
size_t PathKeyHash(const Any& v)
{
    const Material::PathKey& key = v.Get<Material::PathKey>();
    std::hash<String> hashFn;
    return hashFn(key.path.GetAbsolutePathname());
}
} // namespace MaterialAssetLoaderDetail

MaterialAssetLoader::MaterialAssetLoader()
{
    AnyHash<Material::PathKey>::Register(&MaterialAssetLoaderDetail::PathKeyHash);
}

AssetFileInfo MaterialAssetLoader::GetAssetFileInfo(const Any& assetKey) const
{
    DVASSERT(assetKey.CanGet<Material::PathKey>());
    const Material::PathKey& key = assetKey.Get<Material::PathKey>();
    AssetFileInfo info;
    info.fileName = key.path.GetAbsolutePathname();
    if (info.fileName.empty() == true)
    {
        info.inMemoryAsset = true;
    }

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

void MaterialAssetLoader::LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const
{
    Asset<Material> materialAsset = std::dynamic_pointer_cast<Material>(asset);
    if (file == nullptr)
    {
        materialAsset->material = new NMaterial();
        return;
    }
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
    materialAsset->material->PreBuildMaterial(PASS_FORWARD);

    if (archive->IsKeyExists("parentPath"))
    {
        materialAsset->parentPath = serializationContext.GetScenePath() + archive->GetString("parentPath");
        materialAsset->parentAsset = GetEngineContext()->assetManager->GetAsset<Material>(Material::PathKey(materialAsset->parentPath), AssetManager::SYNC);

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

Vector<String> MaterialAssetLoader::GetDependsOnFiles(const AssetBase* asset) const
{
    const Any& assetKey = asset->GetKey();
    DVASSERT(assetKey.CanGet<Material::PathKey>());
    const Material::PathKey& key = assetKey.Get<Material::PathKey>();

    return Vector<String>{ key.path.GetAbsolutePathname() };
}

Vector<const Type*> MaterialAssetLoader::GetAssetKeyTypes() const
{
    return Vector<const Type*>{ Type::Instance<Material::PathKey>() };
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialAssetLoader)
{
}
} // namespace DAVA
