#pragma once

#include "Asset/AbstractAssetLoader.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class GeometryAssetLoader : public AbstractAssetLoader
{
public:
    struct PathKey
    {
        PathKey() = default;
        PathKey(const FilePath& filepath)
            : path(filepath)
        {
        }
        FilePath path;
    };

    GeometryAssetLoader();
    AssetFileInfo GetAssetFileInfo(const Any& assetKey) const override;

    AssetBase* CreateAsset(const Any& assetKey) const override;
    void DeleteAsset(AssetBase* asset) const override;
    void LoadAsset(Asset<AssetBase> asset, File* file, String& errorMessage) const override;
    bool SaveAsset(Asset<AssetBase> asset, File* file, eSaveMode requestedMode) const override;

    bool SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const override;

    Vector<const Type*> GetAssetKeyTypes() const override;
    Vector<const Type*> GetAssetTypes() const override;
};

template <>
bool AnyCompare<GeometryAssetLoader::PathKey>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<GeometryAssetLoader::PathKey>;
} // namespace DAVA
