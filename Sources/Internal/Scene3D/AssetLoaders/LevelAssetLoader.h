#pragma once

#include "Asset/Asset.h"
#include "Asset/AbstractAssetLoader.h"
#include "FileSystem/FilePath.h"
#include "Base/Type.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class Level;
class LevelAssetLoader : public AbstractAssetLoader
{
public:
    LevelAssetLoader();
    AssetFileInfo GetAssetFileInfo(const Any& assetKey) const override;

    AssetBase* CreateAsset(const Any& assetKey) const override;
    void DeleteAsset(AssetBase* asset) const override;
    void LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const override;
    bool SaveAsset(Asset<AssetBase> asset, File* file, eSaveMode requestedMode) const override;

    // Supported data types
    // Scene*
    bool SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const override;

    Vector<const Type*> GetAssetKeyTypes() const override;
    Vector<String> GetDependsOnFiles(const AssetBase* asset) const override;
};

} // namespace DAVA
