#pragma once
#include "Base/BaseTypes.h"
#include "Asset/Asset.h"
#include "Asset/AssetBase.h"
#include "Functional/Function.h"
#include "FileWatcher/FileWatcher.h"
#include "Logger/Logger.h"

namespace DAVA
{
class AssetManager;
LoggerEnable(AssetManager);

class Window;
class AssetManager final
{
public:
    AssetManager();
    ~AssetManager();

    template <class T>
    Asset<T> CreateNewAsset(const FilePath& assetName);

    template <class T>
    Asset<T> LoadAsset(const FilePath& assetName,
                       AssetLoadedFunction assetLoadedCallback = nullptr,
                       bool loadAsyncronously = true);

    template <class T>
    Asset<T> FindAsset(const FilePath& assetName);

    void CollectGarbage();

    bool EnableAssetReloadingOnChange();

    /// Add a directory watch. Same as the other addWatch, but doesn't have recursive option.
    /// For backwards compatibility.
    /// @exception FileNotFoundException Thrown when the requested directory does not exist
    FW::WatchID AddWatch(const String& directory);

    /// Add a directory watch
    /// @exception FileNotFoundException Thrown when the requested directory does not exist
    FW::WatchID AddWatch(const String& directory, bool recursive);

    /// Remove a directory watch. This is a brute force search O(nlogn).
    void RemoveWatch(const String& directory);

    /// Remove a directory watch. This is a map lookup O(logn).
    void RemoveWatch(FW::WatchID watchid);

private:
    void OnAssetChanged(const FilePath& filepath);
    void EnqueueLoadAssetJob(Asset<AssetBase> asset);
    void EnqueuePrepareDataForReloadAssetJob(Asset<AssetBase> asset);

    void LoadAssetJob(Asset<AssetBase> asset);
    void ReloadAssetJob(Asset<AssetBase> asset);
    void PrepareDataForReloadJob(Asset<AssetBase> asset);

    void ReloadAsset();

    Map<FilePath, AssetWeakLink<AssetBase>> cache;

    FW::FileWatcher fileWatcher;
    void OnFWChanged(FW::WatchID watchId, FW::String dir, FW::String filename, FW::Action action);
    void OnAppFocusChanged(Window*, bool visible);
    void OnAppResume();
    void ReloadAllChangedFiles();

    Set<String> changedFiles;
};
}

#include "AssetManager_impl.h"
