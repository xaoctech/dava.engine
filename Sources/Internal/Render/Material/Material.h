#pragma once

#include "Base/Any.h"
#include "Asset/Asset.h"
#include "Asset/AssetListener.h"

#include "Reflection/Reflection.h"

namespace DAVA
{
class NMaterial;
class FilePath;
class Material : public AssetBase
{
public:
    Material(const Any& assetKey);
    ~Material();

    void SetMaterial(NMaterial* material);
    NMaterial* GetMaterial() const;

    void SetParentPath(const FilePath& path);
    const FilePath& GetParentPath() const;

protected:
    friend class MaterialAssetLoader;
    FilePath parentPath;

    //runtime
    Asset<Material> parentAsset = nullptr;
    NMaterial* material = nullptr;
    SimpleAssetListener listener;

    DAVA_VIRTUAL_REFLECTION(Material, AssetBase);
};
};
