#include "Asset/AssetManager.h"
#include "Asset/AssetListener.h"
#include "Asset/AbstractAssetLoader.h"

#include "Concurrency/LockGuard.h"
#include "Debug/DVAssert.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Job/JobManager.h"
#include "Logger/Logger.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Scene3D/AssetLoaders/PrefabAssetLoader.h"
#include "Scene3D/AssetLoaders/GeometryAssetLoader.h"
#include "Scene3D/AssetLoaders/MaterialAssetLoader.h"

#if defined(TRACE_ASSET_REQUESTER)
#include "Debug/Backtrace.h"
#endif

namespace DAVA
{
struct AssetManager::AssetDeleter
{
    void operator()(AssetBase* ptr) const
    {
        GetEngineContext()->assetManager->UnloadAsset(ptr);
    }
};

AssetManager::AssetManager()
{
    Engine::Instance()->endFrame.Connect(MakeFunction(this, &AssetManager::Process));
    RegisterAssetLoader(std::make_unique<PrefabAssetLoader>());
    RegisterAssetLoader(std::make_unique<GeometryAssetLoader>());
    RegisterAssetLoader(std::make_unique<MaterialAssetLoader>());

    fileWatcher.onWatchersChanged.Connect(this, &AssetManager::OnFileEvent);
    loadingThread = Thread::Create(MakeFunction(this, &AssetManager::LoadingThreadFn));
    loadingThread->SetName("AssetManagerThread");
    loadingThread->Start();
}

AssetManager::~AssetManager()
{
    loadQueueAssets.Cancel();
    if (loadingThread->IsJoinable() == true)
    {
        loadingThread->Join();
    }
    else
    {
        while (true)
        {
            Thread::eThreadState state = loadingThread->GetState();
            if (state == Thread::eThreadState::STATE_ENDED ||
                state == Thread::eThreadState::STATE_KILLED)
            {
                break;
            }
        }
    }

    loadQueueAssets.ProcessDeque([this](Deque<AsyncLoadTask>& deque) {
        for (AsyncLoadTask& task : deque)
        {
            Asset<AssetBase> asset = task.asset.lock();
            if (asset != nullptr)
            {
                assetMap.erase(asset->assetKey);
            }
        }

        deque.clear();
    });

    for (const LoadedAssetNode& node : loadedAssets)
    {
        Asset<AssetBase> asset = node.asset.lock();
        if (asset != nullptr)
        {
            assetMap.erase(asset->assetKey);
        }
    }

    DVASSERT(assetMap.empty());

    Set<AbstractAssetLoader*> loaders;
    for (auto node : keyTypeToLoader)
    {
        loaders.insert(node.second);
    }
    keyTypeToLoader.clear();

    for (auto node : assetTypeToLoader)
    {
        loaders.insert(node.second);
    }
    assetTypeToLoader.clear();

    for (AbstractAssetLoader* loader : loaders)
    {
        delete loader;
    }
}

void AssetManager::AddResourceFolder(const String& path)
{
    fileWatcher.Add(path, true);
}

void AssetManager::RemoveResourceFolder(const String& path)
{
    fileWatcher.Remove(path);
}

void AssetManager::SetResourceRoot(const String& path)
{
    if (projectRoot.empty() == false)
    {
        fileWatcher.Remove(projectRoot);
    }

    projectRoot = FilePath(path).GetDirectory().GetAbsolutePathname();
    if (projectRoot.empty() == false)
    {
        fileWatcher.Add(projectRoot, true);
    }
}

void AssetManager::RegisterListener(AssetListener* listener, const Type* assetType)
{
    DVASSERT(listener != nullptr);

    Vector<AssetListener*>& listeners = typeListeners[assetType];
#if defined(__DAVAENGINE_DEBUG__)
    auto listenerIter = std::find(listeners.begin(), listeners.end(), listener);
    DVASSERT(listenerIter == listeners.end());
#endif
    listeners.push_back(listener);
}

void AssetManager::RegisterListener(const Asset<AssetBase>& asset, AssetListener* listener)
{
    if (asset == nullptr)
    {
        return;
    }

    LockGuard<Mutex> guard(assetMapMutex);
    auto iter = assetMap.find(asset->assetKey);
    DVASSERT(iter != assetMap.end());

    AssetNode& assetNode = iter->second;
    if (std::binary_search(assetNode.listeners.begin(), assetNode.listeners.end(), listener) == false)
    {
        listener->instanceListener = true;
        assetNode.listeners.push_back(listener);
        std::sort(assetNode.listeners.begin(), assetNode.listeners.end());
    }
}

void AssetManager::UnregisterListener(const AssetListener* listener)
{
    DVASSERT(listener != nullptr);

    for (auto iter = typeListeners.begin(); iter != typeListeners.end(); ++iter)
    {
        Vector<AssetListener*>& listeners = iter->second;
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }

    if (listener->instanceListener == true)
    {
        LockGuard<Mutex> guard(assetMapMutex);
        for (auto& mapNode : assetMap)
        {
            AssetNode& assetNode = mapNode.second;
            if (std::binary_search(assetNode.listeners.begin(), assetNode.listeners.end(), listener))
            {
                assetNode.listeners.erase(std::remove(assetNode.listeners.begin(), assetNode.listeners.end(), listener), assetNode.listeners.end());
                std::sort(assetNode.listeners.begin(), assetNode.listeners.end());
            }
        }
    }
}

void AssetManager::UnregisterListener(const Asset<AssetBase>& asset, const AssetListener* listener)
{
    if (asset == nullptr)
    {
        return;
    }

    LockGuard<Mutex> guard(assetMapMutex);
    auto iter = assetMap.find(asset->assetKey);
    DVASSERT(iter != assetMap.end());

    AssetNode& assetNode = iter->second;
    assetNode.listeners.erase(std::remove(assetNode.listeners.begin(), assetNode.listeners.end(), listener), assetNode.listeners.end());
    std::sort(assetNode.listeners.begin(), assetNode.listeners.end());
}

void AssetManager::RegisterAssetLoader(std::unique_ptr<AbstractAssetLoader>&& loader)
{
#if defined(__DAVAENGINE_DEBUG__)
    {
        LockGuard<Mutex> guard(assetMapMutex);
        DVASSERT(assetMap.empty(), "All AssetLoaders should be registered at application start");
    }
#endif

    AbstractAssetLoader* assetLoader = loader.release();
    DVASSERT(assetLoader != nullptr);
    Vector<const Type*> types = assetLoader->GetAssetTypes();
    for (const Type* assetType : types)
    {
        DVASSERT(assetTypeToLoader.find(assetType) == assetTypeToLoader.end());
        assetTypeToLoader[assetType] = assetLoader;
    }

    Vector<const Type*> keyTypes = assetLoader->GetAssetKeyTypes();
    for (const Type* keyType : keyTypes)
    {
        keyTypeToLoader[keyType] = assetLoader;
    }
}

Asset<AssetBase> AssetManager::GetAsset(const Any& assetKey, LoadingMode mode, AssetListener* listener)
{
    AbstractAssetLoader* loader = GetAssetLoader(assetKey);
    Asset<AssetBase> asset;

    bool loadFallback = false;

    { // assetMapMutex.Lock()
        LockGuard<Mutex> guad(assetMapMutex);

        auto iter = assetMap.find(assetKey);
        if (iter != assetMap.end())
        {
            AssetNode& assetNode = iter->second;
#if defined(TRACE_ASSET_REQUESTER)
            Vector<void*> backtrace = Debug::GetBacktrace();
            assetNode.backtrace.push_back(Debug::GetBacktraceString(backtrace));
#endif
            asset = assetNode.asset.lock();
            if (asset == nullptr)
            {
                AbstractAssetLoader* loader = GetAssetLoader(assetKey);
                asset = Asset<AssetBase>(loader->CreateAsset(assetKey), AssetDeleter());
                asset->state = AssetBase::QUEUED;
                assetNode.asset = asset;
                assetNode.instanceCount++;
                loadFallback = true;
            }
            else
            {
                if (asset->state == AssetBase::QUEUED && mode == AssetManager::SYNC)
                {
                    bool inQueue = false;
                    loadQueueAssets.ProcessDeque([&asset, &inQueue](Deque<AsyncLoadTask>& deque) {
                        auto iter = std::find_if(deque.begin(), deque.end(), [&asset](const AsyncLoadTask& task) {
                            return task.asset.lock() == asset;
                        });

                        if (iter != deque.end())
                        {
                            inQueue = true;
                            deque.erase(iter);
                        }
                    });

                    if (inQueue == true)
                    {
                        loadFallback = true;
                    }
                    else
                    {
                        while (asset->state == AssetBase::QUEUED)
                        {
                            Thread::Yield();
                        }
                    }
                }
            }

            if (listener != nullptr)
            {
                bool found = std::binary_search(assetNode.listeners.begin(), assetNode.listeners.end(), listener);
                if (found == false)
                {
                    assetNode.listeners.push_back(listener);
                    listener->instanceListener = true;
                    std::sort(assetNode.listeners.begin(), assetNode.listeners.end());
                }

                if (asset->state == AssetBase::LOADED)
                {
                    listener->OnAssetLoaded(asset);
                }
            }
        }
        else
        {
            DVASSERT(asset == nullptr);

            loadFallback = true;
            asset = Asset<AssetBase>(loader->CreateAsset(assetKey), AssetDeleter());
            asset->state = AssetBase::QUEUED;

            AssetNode node;
#if defined(TRACE_ASSET_REQUESTER)
            Vector<void*> backtrace = Debug::GetBacktrace();
            node.backtrace.push_back(Debug::GetBacktraceString(backtrace));
#endif
            node.asset = asset;
            if (listener != nullptr)
            {
                node.listeners.push_back(listener);
            }

            assetMap.emplace(assetKey, node);
        }
    } // assetMapMutex.Unlock()

    if (loadFallback == true)
    {
        if (mode == ASYNC)
        {
            CreateAsyncLoadingTask(asset, loader, false);
        }
        else
        {
            String errorMsg;
            SyncLoadAsset(asset, loader, false, errorMsg);

            if (errorMsg.empty() == true)
            {
                GetEngineContext()->jobManager->CreateMainJob([this, asset]() {
                    NotifyLoaded(asset);
                });
            }
            else
            {
                GetEngineContext()->jobManager->CreateMainJob([this, asset, errorMsg]() {
                    NotifyError(asset, false, errorMsg);
                });
            }
        }
    }

    return asset;
}
Asset<AssetBase> AssetManager::FindAsset(const Any& assetKey)
{
    LockGuard<Mutex> guard(assetMapMutex);
    auto iter = assetMap.find(assetKey);
    if (iter != assetMap.end())
    {
        Asset<AssetBase> asset = iter->second.asset.lock();
        return asset;
    }

    return nullptr;
}

Asset<AssetBase> AssetManager::CreateAsset(const Any& assetKey)
{
    LockGuard<Mutex> guard(assetMapMutex);
    if (assetMap.find(assetKey) != assetMap.end())
    {
        return Asset<AssetBase>();
    }

    AbstractAssetLoader* loader = GetAssetLoader(assetKey);

    Asset<AssetBase> asset(loader->CreateAsset(assetKey), AssetDeleter());
    asset->state = AssetBase::EMPTY;
    AssetNode& node = assetMap[assetKey];
    node.asset = asset;
#if defined(TRACE_ASSET_REQUESTER)
    Vector<void*> backtrace = Debug::GetBacktrace();
    node.backtrace.push_back(Debug::GetBacktraceString(backtrace));
#endif
    return asset;
}

bool AssetManager::SaveAsset(const Asset<AssetBase>& asset, AbstractAssetLoader::eSaveMode saveMode)
{
    Any assetKey = asset->assetKey;
#if defined(__DAVAENGINE_DEBUG__)
    {
        LockGuard<Mutex> guard(assetMapMutex);
        DVASSERT(assetMap.find(assetKey) != assetMap.end());
    }
#endif
    AbstractAssetLoader* loader = GetAssetLoader(assetKey);
    AssetFileInfo fileInfo = loader->GetAssetFileInfo(assetKey);
    if (fileInfo.IsValid() == false)
    {
        return false;
    }

    RefPtr<File> file(File::Create(fileInfo.fileName, File::CREATE | File::WRITE));
    if (file.Get() == nullptr)
    {
        Logger::Error("[AssetManager] Can't open file for save asset %s", fileInfo.fileName.c_str());
        return false;
    }

    bool result = loader->SaveAsset(asset, file.Get(), saveMode);
    if (result == true)
    {
        asset->state = AssetBase::LOADED;
        GetEngineContext()->jobManager->CreateMainJob([this, asset]() {
            NotifyLoaded(asset);
        });
    }

    return result;
}

bool AssetManager::SaveAssetFromData(const Any& saveInfo, const Any& assetKey, AbstractAssetLoader::eSaveMode saveMode)
{
    AbstractAssetLoader* loader = GetAssetLoader(assetKey);
    AssetFileInfo fileInfo = loader->GetAssetFileInfo(assetKey);
    if (fileInfo.IsValid() == false)
    {
        return false;
    }

    RefPtr<File> file(File::Create(fileInfo.fileName, File::CREATE | File::WRITE));
    if (file.Get() == nullptr)
    {
        Logger::Error("[AssetManager] Can't open file for save asset %s", fileInfo.fileName.c_str());
        return false;
    }

    return loader->SaveAssetFromData(saveInfo, file.Get(), saveMode);
}

void AssetManager::ReloadAsset(const Any& assetKey)
{
    bool applyReloading = false;
    Asset<AssetBase> originalAsset;
    {
        LockGuard<Mutex> guard(assetMapMutex);
        auto iter = assetMap.find(assetKey);
        if (iter == assetMap.end())
        {
            return;
        }

        AssetNode& node = iter->second;
        originalAsset = node.asset.lock();
        if (originalAsset != nullptr)
        {
            applyReloading = true;
            originalAsset->state = AssetBase::OUT_OF_DATE;
            node.instanceCount++;
        }
    }

    if (applyReloading == true)
    {
        AbstractAssetLoader* loader = GetAssetLoader(assetKey);
        Asset<AssetBase> asset = Asset<AssetBase>(loader->CreateAsset(assetKey), AssetDeleter());
        asset->state = AssetBase::QUEUED;
        CreateAsyncLoadingTask(asset, loader, true);
        LockGuard<Mutex> guard(loadedMutex);
        toReloadAssets[assetKey] = asset;
    }
}

AssetFileInfo AssetManager::GetAssetFileInfo(const Asset<AssetBase>& asset) const
{
#if defined(__DAVAENGINE_DEBUG__)
    {
        LockGuard<Mutex> guard(const_cast<AssetManager*>(this)->assetMapMutex);
        DVASSERT(assetMap.find(asset->assetKey) != assetMap.end());
    }
#endif
    AbstractAssetLoader* loader = GetAssetLoader(asset->assetKey);
    return loader->GetAssetFileInfo(asset->assetKey);
}

void AssetManager::UnloadAsset(AssetBase* ptr)
{
    Any assetKey = ptr->assetKey;
    {
        LockGuard<Mutex> guard(loadedMutex);
        reloadRequests.erase(assetKey);
        toReloadAssets.erase(assetKey);
    }

    GetEngineContext()->jobManager->CreateMainJob([this, ptr]() {
        bool sendNotify = false;
        {
            LockGuard<Mutex> guard(assetMapMutex);
            auto iter = assetMap.find(ptr->assetKey);
            DVASSERT(iter != assetMap.end());
            iter->second.instanceCount--;
            if (iter->second.instanceCount == 0)
            {
                sendNotify = true;
                assetMap.erase(iter);
            }
        }

        if (sendNotify == true)
        {
            NotifyUnloaded(ptr);
        }
        delete ptr;
    });
}

void AssetManager::NotifyLoaded(const Asset<AssetBase>& asset)
{
    AddAssetFiles(asset.get());
    Notify(asset.get(), true, [&](AssetListener* listener) {
        listener->OnAssetLoaded(asset);
    });
}

void AssetManager::NotifyReloaded(const Asset<AssetBase>& original, const Asset<AssetBase>& reloaded)
{
    RemoveAssetFiles(original.get());
    AddAssetFiles(reloaded.get());
    Notify(reloaded.get(), true, [&](AssetListener* listener) {
        listener->OnAssetReloaded(original, reloaded);
    });
}

void AssetManager::NotifyError(const Asset<AssetBase>& asset, bool reloaded, const String& msg)
{
    AddAssetFiles(asset.get());
    Notify(asset.get(), true, [&](AssetListener* listener) {
        listener->OnAssetError(asset, reloaded, msg);
    });
}

void AssetManager::NotifyUnloaded(AssetBase* asset)
{
    RemoveAssetFiles(asset);

    Notify(asset, false, [&](AssetListener* listener) {
        listener->OnAssetUnloaded(asset);
    });
}

void AssetManager::Notify(AssetBase* asset, bool notifyInstance, const Function<void(AssetListener*)>& callback)
{
    if (notifyInstance == true)
    {
        // Per instance notification
        LockGuard<Mutex> guard(assetMapMutex);
        auto iter = assetMap.find(asset->assetKey);
        DVASSERT(iter != assetMap.end());

        for (AssetListener* listener : iter->second.listeners)
        {
            callback(listener);
        }
    }

    {
        // Per type notification
        const ReflectedType* assetRefType = ReflectedTypeDB::GetByPointer(asset);
        DVASSERT(assetRefType != nullptr);

        const Type* assetType = assetRefType->GetType();
        auto typeIter = typeListeners.find(assetType);
        if (typeIter != typeListeners.end())
        {
            for (AssetListener* listener : typeIter->second)
            {
                callback(listener);
            }
        }
    }

    {
        // Common notification
        auto commonIter = typeListeners.find(nullptr);
        if (commonIter != typeListeners.end())
        {
            for (AssetListener* listener : commonIter->second)
            {
                callback(listener);
            }
        }
    }
}

void AssetManager::AddAssetFiles(const AssetBase* asset)
{
    AbstractAssetLoader* loader = GetAssetLoader(asset->assetKey);
    Vector<String> pathes = loader->GetDependsOnFiles(asset);
    for (const String& path : pathes)
    {
        assetFileMap[path].emplace(asset->assetKey);
    }
}

void AssetManager::RemoveAssetFiles(const AssetBase* asset)
{
    AbstractAssetLoader* loader = GetAssetLoader(asset->assetKey);
    Vector<String> pathes = loader->GetDependsOnFiles(asset);

    for (const String& path : pathes)
    {
        auto iter = assetFileMap.find(path);
        DVASSERT(iter != assetFileMap.end());
        iter->second.erase(asset->assetKey);
        if (iter->second.empty() == true)
        {
            assetFileMap.erase(iter);
        }
    }
}

void AssetManager::OnFileEvent(const String& path, FileWatcher::eWatchEvent e)
{
    if (e == FileWatcher::FILE_REMOVED)
    {
        // should we mark this asset as OUT_OF_DATE
        return;
    }

    UnorderedSet<Any> assetKeys;
    {
        auto iter = assetFileMap.find(path);
        if (iter != assetFileMap.end())
        {
            assetKeys = iter->second;
        }
    }

    for (const Any& key : assetKeys)
    {
        LockGuard<Mutex> guard(loadedMutex);
        reloadRequests.emplace(key);
    }
}

void AssetManager::Process()
{
    Vector<Asset<AssetBase>> lockedLoadedAssets;
    Vector<bool> assetReloadRequested;
    Vector<String> assetErrors;
    {
        Vector<LoadedAssetNode> weakLoadedAssets;
        UnorderedSet<Any> reloadRequestsCopy;
        {
            LockGuard<Mutex> guard(loadedMutex);
            weakLoadedAssets = std::move(loadedAssets);
            reloadRequestsCopy = std::move(reloadRequests);
        }

        lockedLoadedAssets.reserve(weakLoadedAssets.size());
        assetReloadRequested.reserve(weakLoadedAssets.size());
        for (const LoadedAssetNode& weakAsset : weakLoadedAssets)
        {
            Asset<AssetBase> lockedAsset = weakAsset.asset.lock();
            if (lockedAsset != nullptr)
            {
                lockedLoadedAssets.push_back(lockedAsset);
                assetReloadRequested.push_back(weakAsset.reloadRequest);
                assetErrors.push_back(weakAsset.errorMsg);
            }
        }

        for (const Any& toReload : reloadRequestsCopy)
        {
            ReloadAsset(toReload);
        }
    }

    for (size_t i = 0; i < lockedLoadedAssets.size(); ++i)
    {
        Asset<AssetBase> asset = lockedLoadedAssets[i];
        const String& error = assetErrors[i];
        if (error.empty() == true)
        {
            if (assetReloadRequested[i] == false)
            {
                NotifyLoaded(asset);
            }
            else
            {
                toReloadAssets.erase(asset->assetKey);
                Asset<AssetBase> originalAsset;
                {
                    LockGuard<Mutex> guard(assetMapMutex);
                    auto iter = assetMap.find(asset->assetKey);
                    DVASSERT(iter != assetMap.end());
                    originalAsset = iter->second.asset.lock();
                    iter->second.asset = asset;
                }
                if (originalAsset != nullptr)
                {
                    NotifyReloaded(originalAsset, asset);
                }
            }
        }
        else
        {
            NotifyError(asset, assetReloadRequested[i], error);
        }
    }
}

AbstractAssetLoader* AssetManager::GetAssetLoader(const Any& assetKey) const
{
    const Type* keyType = assetKey.GetType();

    auto loaderIter = keyTypeToLoader.find(keyType);
    DVASSERT(loaderIter != keyTypeToLoader.end());
    return loaderIter->second;
}

RefPtr<File> AssetManager::CreateAssetFile(const Any& assetKey, AbstractAssetLoader* loader, String& errorMsg)
{
    DVASSERT(loader != nullptr);

    FileSystem* fs = GetEngineContext()->fileSystem;

    AssetFileInfo fileInfo = loader->GetAssetFileInfo(assetKey);
    DVASSERT(fileInfo.IsValid() == true);

    if (fileInfo.inMemoryAsset == true)
    {
        return RefPtr<File>();
    }

    FilePath path(fileInfo.fileName);
    if (fs->Exists(path) == false)
    {
        errorMsg = Format("[AssetManager] Could not find file %s", path.GetAbsolutePathname().c_str());
        return RefPtr<File>();
    }

    RefPtr<File> file(File::Create(path, File::OPEN | File::READ));
    if (file.Get() == nullptr)
    {
        errorMsg = Format("[AssetManager] Could not open file %s", path.GetAbsolutePathname().c_str());
        return RefPtr<File>();
    }

    if (fileInfo.dataLength != AssetFileInfo::FULL_FILE)
    {
        uint64 fileSize = file->GetSize();
        if (fileInfo.dataOffset + fileInfo.dataLength > fileSize)
        {
            errorMsg = Format("[AssetManager] Requested region of file %s is out of range", path.GetAbsolutePathname().c_str());
            return RefPtr<File>();
        }

        DVASSERT(fileInfo.dataLength <= std::numeric_limits<uint32>::max());
        uint32 fileRegionLength = static_cast<uint32>(fileInfo.dataLength);

        Vector<uint8> data;
        data.resize(fileRegionLength);
        file->Seek(fileInfo.dataOffset, File::SEEK_FROM_START);
        uint32 readed = file->Read(data.data(), fileRegionLength);

        if (readed != fileRegionLength)
        {
            errorMsg = "[AssetManager] Requested part of asset file can't be readed";
            return RefPtr<File>();
        }
        file = RefPtr<File>(DynamicMemoryFile::Create(data.data(), static_cast<int32>(data.size()), File::OPEN | File::READ));
    }
    else
    {
        DVASSERT(fileInfo.dataOffset == 0);
    }

    return file;
}

void AssetManager::CreateAsyncLoadingTask(const Asset<AssetBase>& asset, AbstractAssetLoader* loader, bool reloading)
{
    DVASSERT(asset->state == AssetBase::QUEUED);

    AsyncLoadTask task;
    task.asset = asset;
    task.loader = loader;
    task.reloadRequest = reloading;
    loadQueueAssets.PushBack(task);
}

void AssetManager::SyncLoadAsset(const Asset<AssetBase>& asset, AbstractAssetLoader* loader, bool reloading, String& errorMsg)
{
    RefPtr<File> file(CreateAssetFile(asset->assetKey, loader, errorMsg));
    if (errorMsg.empty() == true)
    {
        loader->LoadAsset(asset, file.Get(), reloading, errorMsg);
    }

    if (errorMsg.empty() == true)
    {
        asset->state = AssetBase::LOADED;
    }
    else
    {
        asset->state = AssetBase::ERROR;
    }
}

void AssetManager::LoadingThreadFn()
{
    while (loadQueueAssets.IsCanceled() == false)
    {
        AsyncLoadTask task = loadQueueAssets.Front(true);
        Asset<AssetBase> lockedAsset = task.asset.lock();
        if (lockedAsset != nullptr)
        {
            String errorMsg;
            SyncLoadAsset(lockedAsset, task.loader, task.reloadRequest, errorMsg);
            AsyncLoadingFinished(task.asset, task.reloadRequest, errorMsg);
        }
    }
}

void AssetManager::AsyncLoadingFinished(const std::weak_ptr<AssetBase>& asset, bool reloading, const String& errorMsg)
{
    LockGuard<Mutex> guard(loadedMutex);

    LoadedAssetNode node;
    node.asset = asset;
    node.reloadRequest = reloading;
    node.errorMsg = errorMsg;
    loadedAssets.push_back(node);
}
}
