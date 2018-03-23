#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"

#include "Asset/Asset.h"
#include "Reflection/Reflection.h"

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
        return fileName.empty() == false || inMemoryAsset == true;
    }
};

class AbstractAssetLoader : public ReflectionBase
{
public:
    enum eSaveMode
    {
        MODE_TEXT,
        MODE_BIN
    };
    virtual ~AbstractAssetLoader() = default;

    virtual AssetFileInfo GetAssetFileInfo(const Any& assetKey) const = 0;
    virtual bool ExistsOnDisk(const Any& assetKey) const;

    virtual AssetBase* CreateAsset(const Any& assetKey) const = 0;
    virtual void DeleteAsset(AssetBase* asset) const = 0;
    virtual void LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const = 0;
    virtual bool SaveAsset(Asset<AssetBase>, File* file, eSaveMode requestedMode) const = 0;
    virtual bool SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const = 0;
    virtual Vector<String> GetDependsOnFiles(const AssetBase* asset) const = 0;

    virtual Vector<const Type*> GetAssetKeyTypes() const = 0;

protected:
    void MofidyAssetKey(Asset<AssetBase> asset, Any&& newKey) const;

    DAVA_VIRTUAL_REFLECTION(AbstractAssetLoader, ReflectionBase);
};
} // namespace DAVA
