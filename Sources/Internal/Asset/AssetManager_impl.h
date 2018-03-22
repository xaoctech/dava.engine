#pragma once

#include <Base/BaseTypes.h>

namespace DAVA
{
template <typename AssetType>
Asset<AssetType> AssetManager::GetAsset(const Any& assetKey, LoadingMode mode, AssetListener* listener)
{
    Asset<AssetBase> asset = GetAsset(assetKey, mode, listener);
    Asset<AssetType> assetTyped = std::dynamic_pointer_cast<AssetType>(asset);
    return assetTyped;
}

template <typename AssetType>
Asset<AssetType> AssetManager::FindAsset(const Any& assetKey)
{
    Asset<AssetBase> asset = FindAsset(assetKey);
    return std::dynamic_pointer_cast<AssetType>(asset);
}

template <typename AssetType>
Asset<AssetType> AssetManager::CreateAsset(const Any& assetKey)
{
    Asset<AssetBase> asset = CreateAsset(assetKey);
    return std::dynamic_pointer_cast<AssetType>(asset);
}

template <typename AssetType>
Asset<AssetType> AssetManager::FindLoadOrCreate(const Any& assetKey)
{
    Asset<AssetBase> asset = FindLoadOrCreate(assetKey);
    return std::dynamic_pointer_cast<AssetType>(asset);
}

template <typename T>
T* AssetManager::GetAssetLoader() const
{
    const Type* requestedType = Type::Instance<T>();
    auto iter = loaderTypeToLoader.find(requestedType);
    if (iter == loaderTypeToLoader.end())
    {
        return nullptr;
    }

    return static_cast<T*>(iter->second);
}
} // namespace DAVA
