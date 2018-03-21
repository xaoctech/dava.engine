#pragma once

#include "Asset/Asset.h"
#include "Asset/AbstractAssetLoader.h"
#include "Render/Material/FXAsset.h"

#include "Concurrency/Mutex.h"
#include "FileSystem/File.h"
#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class FXAssetLoader final : public AbstractAssetLoader
{
public:
    FXAssetLoader();

    AssetFileInfo GetAssetFileInfo(const Any& assetKey) const override;
    bool ExistsOnDisk(const Any& assetKey) const override;

    AssetBase* CreateAsset(const Any& assetKey) const override;
    void DeleteAsset(AssetBase* asset) const override;
    void LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const override;
    bool SaveAsset(Asset<AssetBase>, File* file, eSaveMode requestedMode) const override;
    bool SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const override;
    Vector<String> GetDependsOnFiles(const AssetBase* asset) const override;

    Vector<const Type*> GetAssetKeyTypes() const override;

private:
    FXDescriptor LoadOldTemplate(FastName fxName, FastName quality, bool reloading, String& errorMsg) const;
    FXDescriptor LoadOldTemplateImpl(FastName fxName, FastName quality, bool reloading, String& errorMsg);

    void InitDefaultFx();

    Mutex fxCacheMutex;
    Map<std::pair<FastName, FastName>, FXDescriptor> oldTemplateMap;
    FXDescriptor defaultFx;

    DAVA_VIRTUAL_REFLECTION(FXAssetLoader, AbstractAssetLoader);
};
} // namespace DAVA
