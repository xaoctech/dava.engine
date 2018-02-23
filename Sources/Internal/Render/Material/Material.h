#pragma once

#include "Asset/AssetBase.h"

namespace DAVA
{
class NMaterial;
class FilePath;
class Material : public AssetBase
{
public:
    Material();
    ~Material();

    void SetMaterial(NMaterial* material);
    NMaterial* GetMaterial() const;

    void Load(const FilePath& filepath) override;
    void Save(const FilePath& filepath) override;
    void Reload() override;

    void SetParentPath(const FilePath& path);
    const FilePath& GetParentPath() const;

protected:
    FilePath parentPath;

    //runtime
    Asset<Material> parentAsset = nullptr;
    NMaterial* material = nullptr;
};
};
