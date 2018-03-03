#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"

#include "Asset/Asset.h"

namespace DAVA
{
class File;
struct AssetFileInfo
{
    static const uint64 FULL_FILE;
    // file path or unique name of "in memory asset"
    String fileName;
    uint64 dataLength = FULL_FILE;
    uint64 dataOffset = 0;
    bool inMemoryAsset = false;

    bool IsValid() const
    {
        return fileName.empty() == false;
    }
};

class AbstractAssetLoader
{
public:
    enum eSaveMode
    {
        MODE_TEXT,
        MODE_BIN
    };
    virtual ~AbstractAssetLoader() = default;

    virtual AssetFileInfo GetAssetFileInfo(const Any& assetKey) const = 0;

    virtual AssetBase* CreateAsset(const Any& assetKey) const = 0;
    virtual void DeleteAsset(AssetBase* asset) const = 0;
    virtual void LoadAsset(Asset<AssetBase> asset, File* file, String& errorMessage) const = 0;
    virtual bool SaveAsset(Asset<AssetBase>, File* file, eSaveMode requestedMode) const = 0;
    virtual bool SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const = 0;

    virtual Vector<const Type*> GetAssetKeyTypes() const = 0;
    virtual Vector<const Type*> GetAssetTypes() const = 0;

    virtual AssetFileInfo GetAdditionalLoadFileInfo(String& dependOnFilePath) const;
    virtual Vector<String> GetAdditionalFileExtensionsDependOn() const;
};
} // namespace DAVA
