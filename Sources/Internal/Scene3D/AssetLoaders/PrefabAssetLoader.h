#pragma once

#include "Asset/Asset.h"
#include "Asset/AbstractAssetLoader.h"
#include "FileSystem/FilePath.h"
#include "Base/Type.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class PrefabAssetLoader : public AbstractAssetLoader
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

    PrefabAssetLoader();
    AssetFileInfo GetAssetFileInfo(const Any& assetKey) const override;

    AssetBase* CreateAsset(const Any& assetKey) const override;
    void DeleteAsset(AssetBase* asset) const override;
    void LoadAsset(Asset<AssetBase> asset, File* file, String& errorMessage) const override;
    bool SaveAsset(Asset<AssetBase> asset, File* file, eSaveMode requestedMode) const override;

    // Supported data types
    // Vector<Entity*>
    // Entity*
    // Scene*
    bool SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const override;

    Vector<const Type*> GetAssetKeyTypes() const override;
    Vector<const Type*> GetAssetTypes() const override;
};

template <>
bool AnyCompare<PrefabAssetLoader::PathKey>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<PrefabAssetLoader::PathKey>;

} // namespace DAVA
