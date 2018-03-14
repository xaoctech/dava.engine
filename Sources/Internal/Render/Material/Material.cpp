#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Filesystem/FilePath.h"
#include "Render/Material/Material.h"
#include "Render/Material/NMaterial.h"
#include "Scene3D/AssetLoaders/MaterialAssetLoader.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
Material::Material(const Any& assetKey)
    : AssetBase(assetKey)
{
    listener.onReloaded = [this](const Asset<AssetBase>& original, const Asset<AssetBase>& reloaded) {
        DVASSERT(original == parentAsset);
        parentAsset = std::dynamic_pointer_cast<Material>(reloaded);
    };
}

Material::~Material()
{
    if (parentAsset != nullptr)
    {
        GetEngineContext()->assetManager->UnregisterListener(parentAsset, &listener);
    }
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
    AssetManager* assetManager = GetEngineContext()->assetManager;
    if (parentAsset != nullptr)
    {
        assetManager->UnregisterListener(parentAsset, &listener);
    }

    parentAsset.reset();

    if (parentPath.IsEmpty() == false)
    {
        parentAsset = GetEngineContext()->assetManager->GetAsset<Material>(MaterialAssetLoader::PathKey(parentPath), AssetManager::SYNC, &listener);
        DVASSERT(parentAsset != nullptr);
    }

    if (material != nullptr && parentAsset != nullptr && parentAsset->GetMaterial() != nullptr)
    {
        material->SetParent(parentAsset->GetMaterial());
    }
}

const FilePath& Material::GetParentPath() const
{
    return parentPath;
}

DAVA_VIRTUAL_REFLECTION_IMPL(Material)
{
    ReflectionRegistrator<Material>::Begin()
    .End();
}
};
