#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Filesystem/FilePath.h"
#include "Render/Material/Material.h"
#include "Render/Material/NMaterial.h"
#include "Scene3D/AssetLoaders/MaterialAssetLoader.h"

namespace DAVA
{
Material::Material(const Any& assetKey)
    : AssetBase(assetKey)
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
    parentAsset = GetEngineContext()->assetManager->GetAsset<Material>(MaterialAssetLoader::PathKey(parentPath), false);

    if (material != nullptr && parentAsset != nullptr)
        material->SetParent(parentAsset->GetMaterial());
}

const FilePath& Material::GetParentPath() const
{
    return parentPath;
}

};
