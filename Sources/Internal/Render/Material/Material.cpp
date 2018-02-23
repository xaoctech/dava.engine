#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Filesystem/FilePath.h"
#include "FileSystem/File.h"
#include "FileSystem/YamlParser.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Render/Material/Material.h"
#include "Render/Material/NMaterial.h"

#define MATERIAL_ASSET_YAML_SERIALIZATION 1

namespace DAVA
{
Material::Material()
{
}

Material::~Material()
{
    SafeRelease(material);
}

void Material::SetMaterial(NMaterial* _material)
{
    SafeRelease(material);
    material = SafeRetain(_material);

    if (parentAsset != nullptr)
        material->SetParent(parentAsset->GetMaterial());
}

NMaterial* Material::GetMaterial() const
{
    return material;
}

void Material::SetParentPath(const FilePath& path)
{
    parentPath = path;
    parentAsset = GetEngineContext()->assetManager->LoadAsset<Material>(parentPath);

    if (material != nullptr && parentAsset != nullptr)
        material->SetParent(parentAsset->GetMaterial());
}

const FilePath& Material::GetParentPath() const
{
    return parentPath;
}

void Material::Load(const FilePath& filepath)
{
    SerializationContext serializationContext;
    serializationContext.SetScenePath(filepath.GetDirectory());
    serializationContext.SetVersion(STREAMING_SCENE_VERSION);

    ScopedPtr<KeyedArchive> archive(new KeyedArchive());

    ScopedPtr<YamlParser> yamlParser(YamlParser::Create(filepath));
    if (yamlParser) //try load from yaml-file
    {
        archive->LoadFromYamlNode(yamlParser->GetRootNode());
    }
    else
    {
        archive->Load(filepath);
    }

    material = new NMaterial();
    material->Load(archive, &serializationContext);

    if (archive->IsKeyExists("parentPath"))
    {
        parentPath = serializationContext.GetScenePath() + archive->GetString("parentPath");
        parentAsset = GetEngineContext()->assetManager->LoadAsset<Material>(parentPath);

        material->SetParent(parentAsset->GetMaterial());
    }
}

void Material::Save(const FilePath& filepath)
{
    SerializationContext serializationContext;
    serializationContext.SetScenePath(filepath.GetDirectory());
    serializationContext.SetVersion(STREAMING_SCENE_VERSION);

    if (material != nullptr)
    {
        ScopedPtr<KeyedArchive> archive(new KeyedArchive());

        material->Save(archive, &serializationContext);
        archive->DeleteKey(NMaterialSerializationKey::ParentMaterialKey);

        if (!parentPath.IsEmpty())
            archive->SetString("parentPath", parentPath.GetRelativePathname(serializationContext.GetScenePath()));

#if MATERIAL_ASSET_YAML_SERIALIZATION
        archive->SaveToYamlFile(filepath);
#else
        archive->Save(filepath);
#endif
    }
}

void Material::Reload()
{
    //TODO
    DVASSERT(false);
}
};
