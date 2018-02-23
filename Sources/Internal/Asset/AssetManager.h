#pragma once

#include <Base/BaseTypes.h>
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
                       bool loadAsyncronously = false);

    template <class T>
    Asset<T> FindAsset(const FilePath& assetName);

    void CollectGarbage();

    bool EnableAssetReloadingOnChange();

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
