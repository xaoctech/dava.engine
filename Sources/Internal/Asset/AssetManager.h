#pragma once

#include "Base/BaseTypes.h"
#include "Asset/Asset.h"
#include "Asset/AbstractAssetLoader.h"
#include "Concurrency/Mutex.h"
#include "Functional/Function.h"

namespace DAVA
{
class AssetListener;
class AssetManager final
{
public:
    AssetManager();
    ~AssetManager();
    void SetResourceRoot(String& path);

    void RegisterListener(AssetListener* listener, const Type* assetType);
    void UnregisterListener(AssetListener* listener);

    void RegisterAssetLoader(AbstractAssetLoader* loader);
    void UnregisterAssetLoader(AbstractAssetLoader* loader);

    template <typename AssetType>
    Asset<AssetType> GetAsset(const Any& assetKey, bool asyncLoading, AssetListener* listener = nullptr);
    Asset<AssetBase> GetAsset(const Any& assetKey, bool asyncLoading, AssetListener* listener = nullptr);

    template <typename AssetType>
    Asset<AssetType> FindAsset(const Any& assetKey);
    Asset<AssetBase> FindAsset(const Any& assetKey);

    template <typename AssetType>
    Asset<AssetType> CreateAsset(const Any& assetKey);
    Asset<AssetBase> CreateAsset(const Any& assetKey);

    bool SaveAssetFromData(const Any& saveInfo, const Any& assetKey, AbstractAssetLoader::eSaveMode saveMode = AbstractAssetLoader::eSaveMode::MODE_BIN);
    bool SaveAsset(const Asset<AssetBase>& asset, AbstractAssetLoader::eSaveMode saveMode = AbstractAssetLoader::eSaveMode::MODE_BIN);
    void ReloadAsset(const Any& assetKey);

    AssetFileInfo GetAssetFileInfo(const Asset<AssetBase>& asset) const;

private:
    Asset<AssetBase> LoadAsset(const Any& assetKey, bool asyncLoading, AssetListener* listener, bool reloading);
    AbstractAssetLoader* GetAssetLoader(const Any& assetKey) const;

    void UnloadAsset(AssetBase* ptr);
    void NotifyLoaded(Asset<AssetBase> asset, bool reloaded);
    void NotifyError(Asset<AssetBase> asset, bool reloaded, const String& msg);
    void NotifyUnloaded(Asset<AssetBase> asset);
    void Notify(Asset<AssetBase> asset, const Function<void(AssetListener*)>& callback);

    struct AssetDeleter;

    // "second" type depend on "first" type
    UnorderedMap<const Type*, Vector<const Type*>> dependencyMap;

    UnorderedMap<const Type*, AbstractAssetLoader*> keyTypeToLoader;
    UnorderedMap<const Type*, AbstractAssetLoader*> assetTypeToLoader;
    UnorderedMap<const Type*, Vector<AssetListener*>> typeListeners;

    struct AssetNode
    {
        Vector<AssetListener*> listeners;
        std::weak_ptr<AssetBase> asset;
    };

    UnorderedMap<Any, AssetNode> assetMap;
};
} // namespace DAVA

#include "AssetManager_impl.h"
