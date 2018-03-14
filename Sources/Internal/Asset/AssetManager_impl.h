#pragma once

#include <Base/BaseTypes.h>

namespace DAVA
{
template <typename AssetType>
Asset<AssetType> AssetManager::GetAsset(const Any& assetKey, LoadingMode mode, AssetListener* listener)
{
    Asset<AssetBase> asset = GetAsset(assetKey, mode, listener);
    return std::dynamic_pointer_cast<AssetType>(asset);
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
} // namespace DAVA
