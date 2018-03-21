#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Asset/Asset.h"
#include "Asset/AbstractAssetLoader.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/ConcurrentDeque.h"
#include "Functional/Function.h"
#include "FileSystem/FileWatcher.h"

#define TRACE_ASSET_REQUESTER

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
    void AddResourceFolder(const String& path);
    void RemoveResourceFolder(const String& path);
    void SetResourceRoot(const String& path);

    void RegisterListener(AssetListener* listener, const Type* assetType);
    void RegisterListener(const Asset<AssetBase>& asset, AssetListener* listener);
    void UnregisterListener(const AssetListener* listener);
    void UnregisterListener(const Asset<AssetBase>& asset, const AssetListener* listener);

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

    template <typename AssetType>
    Asset<AssetType> FindLoadOrCreate(const Any& assetKey);
    Asset<AssetBase> FindLoadOrCreate(const Any& assetKey);

    bool ExistsOnDisk(const Any& assetKey);

    bool SaveAssetFromData(const Any& saveInfo, const Any& assetKey, AbstractAssetLoader::eSaveMode saveMode = AbstractAssetLoader::eSaveMode::MODE_BIN);
    bool SaveAsset(const Asset<AssetBase>& asset, AbstractAssetLoader::eSaveMode saveMode = AbstractAssetLoader::eSaveMode::MODE_BIN);
    void ReloadAsset(const Any& assetKey);

    AssetFileInfo GetAssetFileInfo(const Asset<AssetBase>& asset) const;

    template <typename T>
    T* GetAssetLoader() const;

private:
    void OnFileEvent(const String& path, FileWatcher::eWatchEvent e);
    void Process();
    void UnloadAsset(AssetBase* ptr);

    AbstractAssetLoader* GetAssetLoader(const Any& assetKey) const;

    RefPtr<File> CreateAssetFile(const Any& assetKey, AbstractAssetLoader* loader, String& errorMsg);
    void CreateAsyncLoadingTask(const Asset<AssetBase>& asset, AbstractAssetLoader* loader, bool reloading);
    void SyncLoadAsset(const Asset<AssetBase>& asset, AbstractAssetLoader* loader, bool reloading, String& errorMsg);

    void NotifyLoaded(const Asset<AssetBase>& asset);
    void NotifyReloaded(const Asset<AssetBase>& original, const Asset<AssetBase>& reloaded);
    void NotifyError(const Asset<AssetBase>& asset, bool reloaded, const String& msg);
    void NotifyUnloaded(AssetBase* asset);
    void Notify(AssetBase* asset, bool notifyInstance, const Function<void(AssetListener*)>& callback);

    void AddAssetFiles(const AssetBase* asset);
    void RemoveAssetFiles(const AssetBase* asset);

    void LoadingThreadFn();
    void AsyncLoadingFinished(const std::weak_ptr<AssetBase>& asset, bool reloading, const String& errorMsg);

    struct AssetDeleter;

    UnorderedMap<const Type*, AbstractAssetLoader*> keyTypeToLoader;
    UnorderedMap<const Type*, AbstractAssetLoader*> loaderTypeToLoader;
    UnorderedMap<const Type*, Vector<AssetListener*>> typeListeners;

    UnorderedMap<String, UnorderedSet<Any>> assetFileMap;

    struct AssetNode
    {
        Vector<AssetListener*> listeners;
        std::weak_ptr<AssetBase> asset;
        int32 instanceCount = 1;
#if defined(TRACE_ASSET_REQUESTER)
        Vector<String> backtrace;
#endif
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
    UnorderedSet<Any> reloadRequests;
    UnorderedMap<Any, Asset<AssetBase>> toReloadAssets;

    Thread* loadingThread = nullptr;

    FileWatcher fileWatcher;
    String projectRoot;
};
} // namespace DAVA

#include "AssetManager_impl.h"
