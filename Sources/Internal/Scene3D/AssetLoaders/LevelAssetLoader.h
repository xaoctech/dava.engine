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
    struct FullLevelKey
    {
        FullLevelKey() = default;
        FullLevelKey(const FilePath& filepath_)
            : path(filepath_)
        {
        }
        FilePath path;
    };

    struct StreamLevelKey
    {
        StreamLevelKey() = default;
        StreamLevelKey(const FilePath& filepath_)
            : path(filepath_)
        {
        }
        FilePath path;
        RefPtr<File> file;
    };

    struct StreamEntityKey
    {
        StreamEntityKey() = default;
        StreamEntityKey(const Asset<Level>& level_, uint32 entityIndex_)
            : level(level_)
            , entityIndex(entityIndex_)
        {
        }
        Asset<Level> level;
        uint32 entityIndex;
    };

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

template <>
bool AnyCompare<LevelAssetLoader::FullLevelKey>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<LevelAssetLoader::FullLevelKey>;

template <>
bool AnyCompare<LevelAssetLoader::StreamLevelKey>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<LevelAssetLoader::StreamLevelKey>;

template <>
bool AnyCompare<LevelAssetLoader::StreamEntityKey>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<LevelAssetLoader::StreamEntityKey>;

} // namespace DAVA
