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
    RegisterAssetLoader(new PrefabAssetLoader());
    RegisterAssetLoader(new GeometryAssetLoader());
    RegisterAssetLoader(new MaterialAssetLoader());
}

AssetManager::~AssetManager()
{
    DVASSERT(assetMap.empty());
    dependencyMap.clear();

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

void AssetManager::SetResourceRoot(String& path)
{
    // TBD
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

void AssetManager::UnregisterListener(AssetListener* listener)
{
    DVASSERT(listener != nullptr);

    for (auto iter = typeListeners.begin(); iter != typeListeners.end(); ++iter)
    {
        Vector<AssetListener*>& listeners = iter->second;
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }

    if (listener->instanceListener == true)
    {
        for (auto mapNode : assetMap)
        {
            AssetNode& assetNode = mapNode.second;
            if (std::binary_search(assetNode.listeners.begin(), assetNode.listeners.end(), listener))
            {
                assetNode.listeners.erase(std::remove(assetNode.listeners.begin(), assetNode.listeners.end(), listener));
            }
        }
    }
}

void AssetManager::RegisterAssetLoader(AbstractAssetLoader* loader)
{
    DVASSERT(assetMap.empty(), "All AssetLoaders should be registered at application start");
    DVASSERT(loader != nullptr);
    Vector<const Type*> types = loader->GetAssetTypes();
    for (const Type* assetType : types)
    {
        DVASSERT(dependencyMap.find(assetType) == dependencyMap.end());
        DVASSERT(assetTypeToLoader.find(assetType) == assetTypeToLoader.end());

        assetTypeToLoader[assetType] = loader;
        dependencyMap[assetType] = loader->GetDependOnAssetTypes(assetType);
    }

    Vector<const Type*> keyTypes = loader->GetAssetKeyTypes();
    for (const Type* keyType : keyTypes)
    {
        keyTypeToLoader[keyType] = loader;
    }
}

void AssetManager::UnregisterAssetLoader(AbstractAssetLoader* loader)
{
    DVASSERT(loader != nullptr);
}

Asset<AssetBase> AssetManager::GetAsset(const Any& assetKey, bool asyncLoading, AssetListener* listener)
{
    auto iter = assetMap.find(assetKey);
    if (iter != assetMap.end())
    {
        AssetNode& assetNode = iter->second;
        if (listener != nullptr)
        {
            bool found = std::binary_search(assetNode.listeners.begin(), assetNode.listeners.end(), listener);
            if (found == false)
            {
                assetNode.listeners.push_back(listener);
                listener->instanceListener = true;
                std::sort(assetNode.listeners.begin(), assetNode.listeners.end());
            }
        }

        Asset<AssetBase> asset = assetNode.asset.lock();
        DVASSERT(asset != nullptr);
        return asset;
    }

    return LoadAsset(assetKey, asyncLoading, listener, false);
}

Asset<AssetBase> AssetManager::FindAsset(const Any& assetKey)
{
    auto iter = assetMap.find(assetKey);
    if (iter != assetMap.end())
    {
        return iter->second.asset.lock();
    }

    return nullptr;
}

Asset<AssetBase> AssetManager::CreateAsset(const Any& assetKey)
{
    DVASSERT(assetMap.find(assetKey) == assetMap.end());

    AbstractAssetLoader* loader = GetAssetLoader(assetKey);

    Asset<AssetBase> asset(loader->CreateAsset(assetKey), AssetDeleter());
    asset->state = AssetBase::EMPTY;
    AssetNode& node = assetMap[assetKey];
    node.asset = asset;
    return asset;
}

bool AssetManager::SaveAsset(const Asset<AssetBase>& asset, AbstractAssetLoader::eSaveMode saveMode)
{
    Any assetKey = asset->assetKey;
    DVASSERT(assetMap.find(assetKey) != assetMap.end());
    AbstractAssetLoader* loader = GetAssetLoader(assetKey);
    AssetFileInfo fileInfo = loader->GetAssetFileInfo(assetKey);
    if (fileInfo.IsValid() == false)
    {
        return false;
    }

    RefPtr<File> file(File::Create(fileInfo.fileName, File::CREATE | File::WRITE));
    if (file.Get() == nullptr)
    {
        Logger::Error("[AssetManager] Can't open file for save asset %s", fileInfo.fileName);
        return false;
    }

    bool result = loader->SaveAsset(asset, file.Get(), saveMode);
    if (result == true)
    {
        asset->state = AssetBase::LOADED;
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
        Logger::Error("[AssetManager] Can't open file for save asset %s", fileInfo.fileName);
        return false;
    }

    return loader->SaveAssetFromData(saveInfo, file.Get(), saveMode);
}

void AssetManager::ReloadAsset(const Any& assetKey)
{
    auto iter = assetMap.find(assetKey);
    if (iter == assetMap.end())
    {
        return;
    }

    Asset<AssetBase> asset = iter->second.asset.lock();
    DVASSERT(asset != nullptr);
    asset->state = AssetBase::OUT_OF_DATE;
    LoadAsset(assetKey, true, nullptr, true);
}

AssetFileInfo AssetManager::GetAssetFileInfo(const Asset<AssetBase>& asset) const
{
    DVASSERT(assetMap.find(asset->assetKey) != assetMap.end());
    AbstractAssetLoader* loader = GetAssetLoader(asset->assetKey);
    return loader->GetAssetFileInfo(asset->assetKey);
}

void AssetManager::UnloadAsset(AssetBase* ptr)
{
    auto iter = assetMap.find(ptr->assetKey);
    DVASSERT(iter != assetMap.end());
    assetMap.erase(iter);

    AbstractAssetLoader* loader = GetAssetLoader(ptr->assetKey);
    loader->DeleteAsset(ptr);
}

void AssetManager::NotifyLoaded(Asset<AssetBase> asset, bool reloaded)
{
    Notify(asset, [&](AssetListener* listener) {
        listener->OnAssetLoaded(asset, reloaded);
    });
}

void AssetManager::NotifyError(Asset<AssetBase> asset, bool reloaded, const String& msg)
{
    Notify(asset, [&](AssetListener* listener) {
        listener->OnAssetError(asset, reloaded, msg);
    });
}

void AssetManager::NotifyUnloaded(Asset<AssetBase> asset)
{
    Notify(asset, [&](AssetListener* listener) {
        listener->OnAssetUnloaded(asset);
    });
}

void AssetManager::Notify(Asset<AssetBase> asset, const Function<void(AssetListener*)>& callback)
{
    {
        // Per instance notification
        auto iter = assetMap.find(asset->assetKey);
        DVASSERT(iter != assetMap.end());

        for (AssetListener* listener : iter->second.listeners)
        {
            callback(listener);
        }
    }

    {
        // Per type notification
        const ReflectedType* assetRefType = ReflectedTypeDB::GetByPointer(asset.get());
        DVASSERT(assetRefType != nullptr);

        const Type* assetType = assetRefType->GetType()->Deref();
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

Asset<AssetBase> AssetManager::LoadAsset(const Any& assetKey, bool asyncLoading, AssetListener* listener, bool reloading)
{
    AbstractAssetLoader* loader = GetAssetLoader(assetKey);

    Asset<AssetBase> asset(loader->CreateAsset(assetKey), AssetDeleter());
    asset->state = AssetBase::EMPTY;

    AssetNode node;
    node.asset = asset;
    if (listener != nullptr)
    {
        node.listeners.push_back(listener);
        listener->instanceListener = true;
    }

    assetMap[assetKey] = node;

    if (asyncLoading == false)
    {
        FileSystem* fs = GetEngineContext()->fileSystem;

        String errorMsg;
        RefPtr<DynamicMemoryFile> loadFile;
        AssetFileInfo fileInfo = loader->GetAssetFileInfo(assetKey);
        DVASSERT(fileInfo.IsValid() == true);
        if (fileInfo.inMemoryAsset == false || fs->Exists(fileInfo.fileName))
        {
            RefPtr<File> file(File::Create(fileInfo.fileName, File::OPEN | File::READ));

            uint64 dataSize = fileInfo.dataLength;
            if (dataSize == AssetFileInfo::FULL_FILE)
            {
                dataSize = file->GetSize();
            }
            file->Seek(fileInfo.dataOffset, File::SEEK_FROM_START);

            Vector<uint8> data;
            data.resize(dataSize);
            uint32 readed = file->Read(data.data(), static_cast<uint32>(dataSize));
            if (readed != dataSize)
            {
                errorMsg = "[AssetManager] Requested part of asset file can't be readed";
            }
            else
            {
                loadFile.Set(DynamicMemoryFile::Create(std::move(data), File::READ, fileInfo.fileName));
            }
        }

        if (errorMsg.empty() == false)
        {
            NotifyError(asset, reloading, errorMsg);
        }
        else
        {
            String errorMsg;
            loader->LoadAsset(asset, loadFile.Get(), errorMsg);
            if (errorMsg.empty() == true)
            {
                asset->state = AssetBase::LOADED;
                NotifyLoaded(asset, reloading);
            }
            else
            {
                asset->state = AssetBase::ERROR;
                NotifyError(asset, reloading, errorMsg);
            }
        }
    }

    return asset;
}

AbstractAssetLoader* AssetManager::GetAssetLoader(const Any& assetKey) const
{
    const Type* keyType = assetKey.GetType();

    auto loaderIter = keyTypeToLoader.find(keyType);
    DVASSERT(loaderIter != keyTypeToLoader.end());
    return loaderIter->second;
}
}
