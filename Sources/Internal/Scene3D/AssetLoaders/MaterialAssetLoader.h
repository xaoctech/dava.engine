#pragma once

#include "Asset/AbstractAssetLoader.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/File.h"
#include "Base/Type.h"
#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class MaterialAssetLoader final : public AbstractAssetLoader
{
public:
    MaterialAssetLoader();
    AssetFileInfo GetAssetFileInfo(const Any& assetKey) const override;

    AssetBase* CreateAsset(const Any& assetKey) const override;
    void DeleteAsset(AssetBase* asset) const override;
    void LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const override;
    bool SaveAsset(Asset<AssetBase> asset, File* file, eSaveMode requestedMode) const override;
    bool SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const override;
    Vector<String> GetDependsOnFiles(const AssetBase* asset) const override;

    Vector<const Type*> GetAssetKeyTypes() const override;

private:
    DAVA_VIRTUAL_REFLECTION(MaterialAssetLoader, AbstractAssetLoader);
};

} // namespace DAVA
