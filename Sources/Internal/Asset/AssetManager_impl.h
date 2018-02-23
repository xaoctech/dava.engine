#pragma once

#include <Base/BaseTypes.h>

namespace DAVA
{
/*
    It's not yet clear how create new should work, and should it be here at all.
 */
template <class T>
Asset<T> AssetManager::CreateNewAsset(const FilePath& filepath_)
{
    AssetDescriptor* assetDescriptor = new AssetDescriptor(filepath_, nullptr);

    auto newlyCreatedAsset = std::make_shared<T>();
    newlyCreatedAsset->SetState(AssetBase::EMPTY);
    newlyCreatedAsset->SetAssetDescriptor(assetDescriptor);

    AssetWeakLink<T> weakLinkToAsset(newlyCreatedAsset);
    cache[filepath_] = weakLinkToAsset;

    return newlyCreatedAsset;
}

template <class T>
Asset<T> AssetManager::LoadAsset(const FilePath& filepath_,
                                 AssetLoadedFunction assetLoadedCallback_,
                                 bool loadAsyncronously)
{
    Asset<T> cachedAsset = FindAsset<T>(filepath_);
    if (cachedAsset != nullptr)
        return cachedAsset;

    AssetDescriptor* assetDescriptor = new AssetDescriptor(filepath_, assetLoadedCallback_);

    auto newlyCreatedAsset = std::make_shared<T>();
    newlyCreatedAsset->SetState(AssetBase::EMPTY);
    newlyCreatedAsset->SetAssetDescriptor(assetDescriptor);

    AssetWeakLink<T> weakLinkToAsset(newlyCreatedAsset);
    cache[filepath_] = weakLinkToAsset;

    if (loadAsyncronously)
    {
        EnqueueLoadAssetJob(newlyCreatedAsset);
    }
    else
    {
        LoadAssetJob(newlyCreatedAsset);
    }
    return newlyCreatedAsset;
}

template <class T>
Asset<T> AssetManager::FindAsset(const FilePath& filepath)
{
    auto it = cache.find(filepath);
    if (it != cache.end())
    {
        AssetWeakLink<AssetBase> base = it->second;
        Asset<AssetBase> assetBase = base.lock();
        if (assetBase)
        {
            Asset<T> assetTyped = std::dynamic_pointer_cast<T>(assetBase);
            return assetTyped;
        }
    }
    return nullptr;
}
}
