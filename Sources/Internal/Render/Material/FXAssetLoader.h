#pragma once

#include "Asset/Asset.h"
#include "Asset/AbstractAssetLoader.h"
#include "Render/Material/FXAsset.h"

#include "Concurrency/Mutex.h"
#include "FileSystem/File.h"
#include "Base/BaseTypes.h"
#include "Base/Any.h"

namespace DAVA
{
class FXAssetLoader : public AbstractAssetLoader
{
public:
    struct Key
    {
        Key() = default;
        Key(const FastName& fxName, const FastName& quality, UnorderedMap<FastName, int32>&& inputDefines);

        FastName fxName;
        FastName quality;
        UnorderedMap<FastName, int32> defines;
        Vector<size_t> fxKey;
        size_t fxKeyHash;
    };

    FXAssetLoader();

    AssetFileInfo GetAssetFileInfo(const Any& assetKey) const override;

    AssetBase* CreateAsset(const Any& assetKey) const override;
    void DeleteAsset(AssetBase* asset) const override;
    void LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const override;
    bool SaveAsset(Asset<AssetBase>, File* file, eSaveMode requestedMode) const override;
    bool SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const override;
    Vector<String> GetDependsOnFiles(const AssetBase* asset) const override;

    Vector<const Type*> GetAssetKeyTypes() const override;
    Vector<const Type*> GetAssetTypes() const override;

private:
    FXDescriptor LoadOldTemplate(FastName fxName, FastName quality, bool reloading, String& errorMsg) const;
    FXDescriptor LoadOldTemplateImpl(FastName fxName, FastName quality, bool reloading, String& errorMsg);

    void InitDefaultFx();

    Mutex fxCacheMutex;
    Map<std::pair<FastName, FastName>, FXDescriptor> oldTemplateMap;
    FXDescriptor defaultFx;
};

template <>
bool AnyCompare<FXAssetLoader::Key>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<FXAssetLoader::Key>;
} // namespace DAVA
