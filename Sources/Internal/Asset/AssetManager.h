#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Asset/Asset.h"
#include "Asset/AbstractAssetLoader.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/ConcurrentDeque.h"
#include "Functional/Function.h"

namespace DAVA
{
class File;
class AssetListener;
class AssetManager final
{
public:
    enum LoadingMode
    {
        SYNC,
        ASYNC
    };

    AssetManager();
    ~AssetManager();
    void SetResourceRoot(String& path);

    void RegisterListener(AssetListener* listener, const Type* assetType);
    void UnregisterListener(AssetListener* listener);
    void UnregisterListener(const Asset<AssetBase>& asset, AssetListener* listener);

    void RegisterAssetLoader(std::unique_ptr<AbstractAssetLoader>&& loader);

    template <typename AssetType>
    Asset<AssetType> GetAsset(const Any& assetKey, LoadingMode mode, AssetListener* listener = nullptr);
    Asset<AssetBase> GetAsset(const Any& assetKey, LoadingMode mode, AssetListener* listener = nullptr);

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
    void Process();
    Asset<AssetBase> GetAsset(const Any& assetKey, LoadingMode mode, AssetListener* listener, bool reloadRequest);
    AbstractAssetLoader* GetAssetLoader(const Any& assetKey) const;

    RefPtr<File> CreateAssetFile(const Any& assetKey, AbstractAssetLoader* loader, String& errorMsg) const;

    void UnloadAsset(AssetBase* ptr);
    void NotifyLoaded(Asset<AssetBase> asset, bool reloaded);
    void NotifyError(Asset<AssetBase> asset, bool reloaded, const String& msg);
    void NotifyUnloaded(AssetBase* asset);
    void Notify(AssetBase* asset, const Function<void(AssetListener*)>& callback, bool notifyInstance = true);

    void LoadingThreadFn();
    void AsyncLoadingFinished(const std::weak_ptr<AssetBase>& asset, bool reloading, const String& errorMsg);

    struct AssetDeleter;

    UnorderedMap<const Type*, AbstractAssetLoader*> keyTypeToLoader;
    UnorderedMap<const Type*, AbstractAssetLoader*> assetTypeToLoader;
    UnorderedMap<const Type*, Vector<AssetListener*>> typeListeners;

    struct AssetNode
    {
        Vector<AssetListener*> listeners;
        std::weak_ptr<AssetBase> asset;
    };

    Mutex assetMapMutex;
    UnorderedMap<Any, AssetNode> assetMap;

    struct AsyncLoadTask
    {
        std::weak_ptr<AssetBase> asset;
        AbstractAssetLoader* loader;
        bool reloadRequest = false;
    };
    ConcurrentDeque<AsyncLoadTask> loadQueueAssets;

    Mutex loadedMutex;
    struct LoadedAssetNode
    {
        LoadedAssetNode() = default;
        LoadedAssetNode(const LoadedAssetNode& other)
            : reloadRequest(other.reloadRequest)
            , asset(other.asset)
            , errorMsg(other.errorMsg)
        {
        }

        LoadedAssetNode(LoadedAssetNode&& other)
            : reloadRequest(other.reloadRequest)
            , asset(std::move(other.asset))
            , errorMsg(std::move(other.errorMsg))
        {
        }

        bool reloadRequest = false;
        std::weak_ptr<AssetBase> asset;
        String errorMsg;
    };
    Vector<LoadedAssetNode> loadedAssets;

    Mutex unloadQueueMutex;
    UnorderedMap<Any, AssetBase*> unloadAssets;

    Thread* loadingThread = nullptr;
};
} // namespace DAVA

#include "AssetManager_impl.h"
